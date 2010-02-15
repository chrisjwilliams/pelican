#ifndef PELICANSERVERCLIENT_H
#define PELICANSERVERCLIENT_H

#include "AbstractDataClient.h"
#include <QDomElement>

/**
 * @file PelicanServerClient.h
 */

namespace pelican {

class AbstractClientProtocol;
class ConfigNode;
class AdapterFactory;

/**
 * @class PelicanServerClient
 *  
 * @brief
 * Implements the data client interface for attaching to a Pelican Server
 *
 * @details
 * 
 */
class PelicanServerClient : public AbstractDataClient
{
    public:
        PelicanServerClient(const ConfigNode& config,
                AdapterFactory* adapterFactory,
                QList<DataRequirements>& dataRequirements);
        ~PelicanServerClient();
        virtual QHash<QString, DataBlob*> getData(QHash<QString, DataBlob*>& dataHash);

    protected:
        void getServiceData(QHash<QString,QString> requirements, QHash<QString, DataBlob*>& datahash);
        void _connect();

    private:
        AbstractClientProtocol* _protocol;
};

} // namespace pelican

#endif // PELICANSERVERCLIENT_H 
