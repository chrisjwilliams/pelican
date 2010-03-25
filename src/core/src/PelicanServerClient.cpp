#include <vector>
#include <QHash>
#include <QTcpSocket>
#include <QBuffer>
#include <QByteArray>
#include <QDebug>
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

    _protocol = new PelicanClientProtocol;

    setIP_Address(config.getOption("server","host"));
    QString port = config.getOption("server","port");
    if( port != "" )
        setPort(port.toUInt());
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
    _server = ipaddress;
}

QHash<QString, DataBlob*> PelicanServerClient::getData(QHash<QString, DataBlob*>& dataHash)
{
    QHash<QString, DataBlob*> validData;
    QSet<QString> reqs = _requireSet;
    if( ! reqs.subtract(QSet<QString>::fromList(dataHash.keys())).isEmpty() )
        throw(QString("PelicanServerClient::getData() data hash does not "
                "contain objects for all possible requests"));

    // construct the request
    StreamDataRequest sr;
    foreach(const DataRequirements& d, dataRequirements())
    {
        sr.addDataOption( d );
    }
    if( ! sr.isEmpty() ) {
        validData.unite( _sendRequest(sr, dataHash) );
    }
    else {
        throw(QString("PelicanServerClient::getData(): "
                "Request for non-stream data"));
    }

    return validData;

}

QHash<QString, DataBlob*> PelicanServerClient::_response(QIODevice& device,
        boost::shared_ptr<ServerResponse> r, QHash<QString, DataBlob*>& dataHash)
{

    QHash<QString, DataBlob*> validData;

    switch( r->type() ) {
        case ServerResponse::Error:
            std::cerr << "PelicanServerClient: Server Error: " << r->message().toStdString() << std::endl;
            break;
        case ServerResponse::StreamData:
            {
                ServiceDataRequest req;
                StreamDataResponse* response = static_cast<StreamDataResponse*>(r.get());
                // determine the associated service data
                StreamData* sd = response->streamData();
                Q_ASSERT( sd != 0 );
                foreach (const boost::shared_ptr<Data>& d, sd->associateData()) {
                    // do we already have it?
                    if( dataHash[d->name()]->version() == d->id())
                    {
                        // no need to fetch it, just mark the existing data as valid
                        validData[d->name()] = dataHash[ d->name()];
                    }
                    else {
                        // need to fetch it from the server
                        req.request( d->name(), d->id());
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
                    std::vector<char> tmp;
                    tmp.resize(sd->size());
                    int timeout = 2000;
                    unsigned long bytesReadTotal = 0;
                    while( bytesReadTotal != sd->size() )
                    {
                        while (device.bytesAvailable() < 1 )
                        {
                            if ( !device.waitForReadyRead(timeout) ) {
                                log(QString("PelicanServerClient: Timed out from server.") + device.errorString() );
                                return validData;
                            }
                        }
                        int bytesRead = device.read(&tmp[0 + bytesReadTotal], sd->size());
                        if(bytesRead == -1) {
                            log(QString("PelicanServerClient: Problem reading from server.") + device.errorString() );
                            return validData;
                        }
                        bytesReadTotal += bytesRead;
                    }
                    Q_ASSERT(tmp.size() == sd->size());
                    // Fetch the service data
                    validData.unite(_getServiceData(req, dataHash));
                    // Now we can adapt the stream data
                    QByteArray tmp_array =  QByteArray::fromRawData(&tmp[0], tmp.size());
                    QBuffer buf(&tmp_array);
                    buf.open(QIODevice::ReadOnly);
                    validData.unite(_adaptStream(buf, sd, dataHash ));
                }
            }
            break;
        case ServerResponse::ServiceData:
            {
                // Service data
                ServiceDataResponse* res = static_cast<ServiceDataResponse*>(r.get());
                foreach( const Data* d, res->data() ) {
                    QString type = d->name();
                    AbstractServiceAdapter* adapter = serviceAdapter(type);
                    Q_ASSERT(adapter != 0 );
                    dataHash[type]->setVersion(d->id());
                    adapter->config(dataHash[type], d->size() );
                    adapter->deserialise(&device);
                    validData.insert(type, dataHash.value(type));
                }
            }
            break;
        default:
            std::cerr << "PelicanServerClient: Unknown Response" << std::endl;
            break;
    }
    return validData;
}

QHash<QString, DataBlob*> PelicanServerClient::_adaptStream(QIODevice& device, const StreamData* sd, QHash<QString, DataBlob*>& dataHash )
{
    QHash<QString, DataBlob*> validData;

    const QString& type = sd->name();
    dataHash[type]->setVersion(sd->id());
    AbstractStreamAdapter* adapter = streamAdapter(type);
    Q_ASSERT( adapter != 0 );
    adapter->config( dataHash[type], sd->size(), dataHash );
    adapter->deserialise(&device);
    validData.insert(type, dataHash.value(type));

    return validData;
}

void PelicanServerClient::_connect()
{
}

QHash<QString, DataBlob*> PelicanServerClient::_getServiceData(const ServiceDataRequest& request, QHash<QString, DataBlob*>& dataHash)
{
    return _sendRequest(request, dataHash);
}


QHash<QString, DataBlob*> PelicanServerClient::_sendRequest(const ServerRequest& request, QHash<QString, DataBlob*>& dataHash)
{
    QHash<QString, DataBlob*> validData;

    QTcpSocket sock;
    Q_ASSERT( _server != "" );
    sock.connectToHost( _server, _port , QIODevice::ReadWrite);
    if( ! sock.waitForConnected(-1) )
    {
        throw(QString("PelicanServerClient: unable to connect to host ") + _server );
    }

    sock.write( _protocol->serialise(request) );
    sock.flush();

    sock.waitForReadyRead();

    boost::shared_ptr<ServerResponse> r = _protocol->receive(sock);
    validData =  _response(sock, r, dataHash);
    std::cout << "-------------------" << std::endl;
    foreach (QString key, validData.keys()) {
        std::cout << "Key: " << key.toStdString() << std::endl;
    }
    std::cout << "-------------------" << std::endl;
    return validData;
}

} // namespace pelican
