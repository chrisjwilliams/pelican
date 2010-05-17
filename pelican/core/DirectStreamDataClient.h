#ifndef DIRECTSTREAMDATACLIENT_H
#define DIRECTSTREAMDATACLIENT_H

#include "pelican/core/AbstractDataClient.h"

/**
 * @file DirectStreamDataClient.h
 */

namespace pelican {

class ChunkerManager;
class DataManager;
class Config;

/**
 * @class DirectStreamDataClient
 *
 * @brief
 * A data client that connects directly to one or more data streams.
 *
 * @details
 * This data client connects directly to one or more data streams using
 * specialist chunkers, rather than to the PelicanServer.
 * This is useful when the data rates on the stream can be handled in a single
 * pipeline.
 *
 * Add the chunkers using the addChunker() method in a derived class
 * constructor.
 */
class DirectStreamDataClient : public AbstractDataClient
{
    public:
        /// Constructs the direct stream data client.
        DirectStreamDataClient(const ConfigNode& configNode,
                const DataTypes& types, const Config* config);

        /// Destroys the direct stream data client.
        virtual ~DirectStreamDataClient();

        /// Obtains a hash of data blobs for the pipeline.
        QHash<QString, DataBlob*> getData(QHash<QString, DataBlob*>& dataHash);

        /// Sets the chunker for the given data type.
        void addChunker(const QString& dataType, const QString& chunkerType,
                const QString& chunkerName = QString());

        /// Sets the service chunker for the given data type.
        void addServiceChunker(const QString& chunkerType,
                const QString& chunkerName = QString());

        /// Sets the stream chunker for the given data type.
        void addStreamChunker(const QString& chunkerType,
                const QString& chunkerName = QString());

    private:
        bool _started;
        ChunkerManager* _chunkerManager;
        DataManager* _dataManager;
};

} // namespace pelican

#endif // DIRECTSTREAMDATACLIENT_H
