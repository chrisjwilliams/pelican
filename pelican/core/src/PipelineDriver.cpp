#include "pelican/core/PipelineDriver.h"

#include "pelican/core/DataClientFactory.h"
#include "pelican/core/AbstractDataClient.h"
#include "pelican/core/FileDataClient.h"
#include "pelican/core/AbstractPipeline.h"
#include "pelican/core/ModuleFactory.h"
#include "pelican/data/DataBlob.h"
#include "pelican/data/DataBlobFactory.h"
#include "pelican/utility/Config.h"
#include "pelican/utility/ConfigNode.h"

#include <QString>
#include <QtGlobal>
#include <QtDebug>
#include <iostream>

#include "pelican/utility/memCheck.h"


namespace pelican {

/**
 * @details
 * PipelineDriver constructor, which takes pointers to the allocated factories.
 */
PipelineDriver::PipelineDriver(DataBlobFactory* blobFactory,
        ModuleFactory* moduleFactory, DataClientFactory* clientFactory
){
    // Initialise member variables.
    _run = false;
    _dataClient = NULL;
    _ignoreEmptyHash = false;

    // Store pointers to factories.
    _blobFactory = blobFactory;
    _moduleFactory = moduleFactory;
    _clientFactory = clientFactory;
    Q_ASSERT(_blobFactory != 0 );
    Q_ASSERT(_moduleFactory != 0 );
    Q_ASSERT(_clientFactory != 0 );
}

/**
 * @details
 * PipelineDriver destructor. This deletes all the registered pipelines
 * and the data blobs.
 */
PipelineDriver::~PipelineDriver()
{
    // Delete the pipelines.
    foreach (AbstractPipeline* pipeline, _registeredPipelines) {
        delete pipeline;
    }
    _registeredPipelines.clear();

    // Delete the data blobs.
    foreach (DataBlob* dataBlob, _dataHash) {
        delete dataBlob;
    }
    _dataHash.clear();

}

/**
 * @details
 * Public interface to register the given \p pipeline with the driver.
 * The pipeline is initialised when this method is called.
 *
 * Registered pipelines will be deleted when the class is destroyed.
 *
 * @param[in] pipeline Pointer to the allocated pipeline.
 */
void PipelineDriver::registerPipeline(AbstractPipeline *pipeline)
{
    // Check the pipeline exists.
    if (!pipeline)
        throw QString("PipelineDriver::registerPipeline(): Null pipeline.");

    // Store the pointer to the registered pipeline.
    _registeredPipelines.append(pipeline);

    // Set up and initialise the pipeline.
    pipeline->setModuleFactory(_moduleFactory);
    pipeline->setPipelineDriver(this);
    pipeline->init();

    // Store the remote data requirements.
    _allDataReq.append(pipeline->requiredDataRemote());
}

/**
 * @details
 * Sets the given data client based on the named argument.
 * The client is only created after the pipelines have been initialised,
 * when start() is called.
 *
 * The recognised values are:
 * - FileDataClient
 * - PelicanServerClient
 *
 * @param[in] name The type of the data client to create.
 */
void PipelineDriver::setDataClient(QString name)
{
    _dataClientName = name;
}

/**
 * @details
 * Iterates over all registered pipelines to determine the required data and
 * starts the data flow through the pipelines.
 *
 * This public method is called by PipelineApplication start().
 */
void PipelineDriver::start()
{
    // Check for at least one registered pipeline.
    if (_registeredPipelines.isEmpty())
        throw QString("PipelineDriver::start(): No pipelines registered.");

    // Store the remote data requirements for each pipeline.
    foreach (AbstractPipeline* pipeline, _registeredPipelines) {
        _pipelines.insert(pipeline->requiredDataRemote(), pipeline);
    }

    // Create data blobs for all pipelines.
    foreach ( DataRequirements req, _allDataReq ) {
        foreach ( QString type, req.allData() ) {
            if ( ! _dataHash.contains(type) )
                _dataHash.insert(type, _blobFactory->create(type));
        }
    }

    // Create the data client.
    if (!_dataClientName.isEmpty())
        _dataClient = _clientFactory->create(_dataClientName, _allDataReq);

    // Check the data requirements.
    _checkDataRequirements();

    // Enter main program loop.
    _run = true;
    while (_run) {
        // Get the data from the client.
        QHash<QString, DataBlob*> validData;
        if (_dataClient)
            validData = _dataClient->getData(_dataHash);

        // Run all the pipelines compatible with this data hash.
        bool ranPipeline = false;
        QMultiHash<DataRequirements, AbstractPipeline*>::iterator pipe;
        for (pipe = _pipelines.begin(); pipe != _pipelines.end(); ++pipe) {
            if (pipe.key().isCompatible(validData)) {
                ranPipeline = true;
                pipe.value()->run(_dataHash);
            }
        }

        // Check if no pipelines were run.
        if (!ranPipeline && !_ignoreEmptyHash)
            throw QString("PipelineDriver::start(): No pipelines run.");
    }
}

/**
 * @details
 * Stops the pipeline driver.
 * This method should be called by a running pipeline.
 */
void PipelineDriver::stop()
{
    _run = false;
}

/**
 * @details
 * Private method to find the data requirements of the given pipeline.
 * This is called by start().
 */
void PipelineDriver::_checkDataRequirements()
{
    if (!_dataClient)
        return;

    /* Check that the set of stream data required for each pipeline does not
     * intersect the set of stream data required by another.
     * Data is not currently copied, so this ensures that two pipelines do not
     * try to modify the same data. */
    DataRequirements totalReq;
    foreach (DataRequirements req, _dataClient->dataRequirements()) {
        if ((totalReq.streamData() & req.streamData()).size() > 0) {
            throw QString("Multiple pipelines requiring the same remote stream data are not supported.");
        }
        totalReq += req;
    }
}

} // namespace pelican
