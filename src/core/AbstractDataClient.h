#ifndef ABSTRACTDATACLIENT_H
#define ABSTRACTDATACLIENT_H

#include "DataTypes.h"
#include "utility/ConfigNode.h"
#include <QHash>
#include <QList>
#include <QSet>
#include <QString>

/**
 * @file AbstractDataClient.h
 */

namespace pelican {

class AbstractAdapter;
class DataBlob;

/**
 * @class AbstractDataClient
 *  
 * @brief
 * This is the abstract base class for the data client used by the pipeline
 * driver.
 * 
 * @details
 * The data client fetches data from the data server and makes it available
 * to the pipelines via the pipeline driver. The data client is created by
 * the pipeline driver after determining the data requirements.
 *
 * Inherit this class and implement the getData() method to create a new data
 * client type.
 *
 *
 *
 * \section Configuration
 *
 * Under its root XML configuration node, each data client must have a list of
 * data types and data adapters, for example:
 *
 * \verbatim
      <data type="VisibilityData" adapter="AdapterStationVisibilities"/>
      <data type="AntennaPositions" adapter="AdapterStationAntennas"/>
 * \endverbatim
 *
 * This information is read by the abstract data client, and stored in the
 * hash accessible using the adapterNames() method.
 */
class AbstractDataClient
{
    private:
        /// The DataTypes and requirements.
        DataTypes _dataReqs;

        /// The configuration node for the data client.
        ConfigNode _configNode;

    protected:
        QSet<QString> _requireSet;

        /// Writes a message to the log.
        void log(const QString& msg);

    protected:
        /// Returns a pointer to the configuration node.
        const ConfigNode& configNode() {return _configNode;}

        /// Returns the adapter for service data of the required type.
        AbstractServiceAdapter* serviceAdapter(const QString& type) const {
            return _dataReqs.serviceAdapter(type);
        }

        /// Returns the adapter for stream data of the required type.
        AbstractStreamAdapter* streamAdapter(const QString& type) const {
            return _dataReqs.streamAdapter(type);
        }

    public:
        /// Data client constructor.
        AbstractDataClient(const ConfigNode& config,
                           const DataTypes& types );

        /// Data client destructor (virtual).
        virtual ~AbstractDataClient();

        /// Returns the list of data requirements for each pipeline.
        const QList<DataRequirements>& dataRequirements() {
            return _dataReqs.dataRequirements();
        }

        /// Gets the requested data from the data server.
        /// This method gets tFills the given data hash, and returns another hash of valid data.
        virtual QHash<QString, DataBlob*> getData(QHash<QString, DataBlob*>&) = 0;
};

} // namespace pelican

#endif // ABSTRACTDATACLIENT_H
