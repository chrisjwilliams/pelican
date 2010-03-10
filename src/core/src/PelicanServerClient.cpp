#include <QHash>
#include <QTcpSocket>
#include <QBuffer>
#include <QByteArray>
#include "PelicanServerClient.h"
#include "adapters/AbstractServiceAdapter.h"
#include "adapters/AbstractStreamAdapter.h"
#include "data/DataRequirements.h"
#include "data/DataBlob.h"
#include "comms/ServerResponse.h"
#include "comms/StreamData.h"
#include "comms/StreamDataRequest.h"
#include "comms/ServiceDataRequest.h"
#include "comms/StreamDataResponse.h"
#include "comms/ServiceDataResponse.h"
#include "comms/PelicanClientProtocol.h"

#include "utility/memCheck.h"

namespace pelican {


// class PelicanServerClient
PelicanServerClient::PelicanServerClient(const ConfigNode& config, const DataTypes& types )
    : AbstractDataClient(config, types ), _protocol(0)
{
    _protocol = new PelicanClientProtocol; // hard code for now
}

PelicanServerClient::~PelicanServerClient()
{
    delete _protocol;
}

void PelicanServerClient::setPort(unsigned int port)
{
    _port = port;
}

void PelicanServerClient::setIP_Address(const QString& ipaddress)
{
    _ipadress = ipaddress;
}

QHash<QString, DataBlob*> PelicanServerClient::getData(QHash<QString, DataBlob*>& dataHash)
{
    QHash<QString, DataBlob*> validData;

    // construct the request
    StreamDataRequest sr;
    sr.addDataOption(dataRequirements());

    // send the request to the server
    QTcpSocket sock;
    //sock.connectToHost( _server, _port );
    //sock.send( _protocol.serialise(sr) );
    //sock.flush();
    //
    // interpret server response
    boost::shared_ptr<ServerResponse> r = _protocol->receive(sock);
    validData.unite( _response(sock, r, dataHash) );

    return validData;

}

QHash<QString, DataBlob*> PelicanServerClient::_response(QIODevice& device, boost::shared_ptr<ServerResponse> r, QHash<QString, DataBlob*>& dataHash)
{

    QHash<QString, DataBlob*> validData;

    switch( r->type() ) {
        case ServerResponse::Error:
            break;
        case ServerResponse::StreamData:
            {
                ServiceDataRequest req;
                StreamDataResponse* res = static_cast<StreamDataResponse*>(r.get());
                // determine the associated service data
                StreamData* sd = res->streamData();
                Q_ASSERT( sd != 0 );
                foreach(const boost::shared_ptr<Data>& d, sd->associateData() ) {
                    // do we already have it?
                    if( dataHash[d->name()]->version() == d->id() )
                    {
                        // no need to fetch it, just mark the existing data as valid
                        validData[d->name()] = dataHash[ d->name() ];
                    }
                    else {
                        // need to fetch it from the server
                        req.request( d->name(), d->id() );
                    }
                }
                // get the stream data
                if( req.isEmpty() ) {
                    // no service data to fetch so we can stream it
                    // straight through the adapter
                    validData.unite(_adaptStream(device, sd, dataHash ));
                }
                else {
                    // we need to buffer the data and fetch the
                    // required service data before we can send it
                    // through the adpater, as the adpapter may require
                    // the contents of the service data to interpret the
                    // incoming data stream
                    QByteArray tmp;
                    tmp.reserve(sd->size() + 1 );
                    device.read( tmp.data() , sd->size() );
                    // fetch trhe service data
                    validData.unite( _getServiceData(req, dataHash) );
                    // now we can adapt the stream data
                    QBuffer buf(&tmp);
                    validData.unite(_adaptStream(buf, sd, dataHash ));
                }
            }
            break;
        case ServerResponse::ServiceData:
            {
                // service data
                ServiceDataResponse* res = static_cast<ServiceDataResponse*>(r.get());
                foreach( const Data* d, res->data() ) {
                    QString type = d->name();
                    AbstractServiceAdapter* adapter = serviceAdapter(type);
                    Q_ASSERT(adapter != 0 );
                    adapter->config(dataHash[type], d->size() );
                    adapter->deserialise(&device);
                    validData.insert(type, dataHash.value(type));
                }
            }
            break;
        default:
            break;
    }
    return validData;
}

QHash<QString, DataBlob*> PelicanServerClient::_adaptStream(QIODevice& device, const StreamData* sd, QHash<QString, DataBlob*>& dataHash )
{
    QHash<QString, DataBlob*> validData;

    const QString& type = sd->name();
    AbstractStreamAdapter* adapter = streamAdapter(type);
    Q_ASSERT( adapter != 0 );
    adapter->config( dataHash[type], sd->size(), dataHash );
    //adapter->deserialise(&device);
    validData.insert(type, dataHash.value(type));

    return validData;
}

void PelicanServerClient::_connect()
{
}

QHash<QString, DataBlob*> PelicanServerClient::_getServiceData(const ServiceDataRequest& requirements, QHash<QString, DataBlob*>& dataHash)
{
    QHash<QString, DataBlob*> validData;

    QTcpSocket sock;
    //sock.connectToHost( _server, _port );
    //sock.send( _protocol.serialise(sr) );
    //sock.flush();
    boost::shared_ptr<ServerResponse> r = _protocol->receive(sock);
    validData =  _response(sock, r, dataHash);
    return validData;
}

} // namespace pelican
