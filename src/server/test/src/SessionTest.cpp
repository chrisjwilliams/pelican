#include "SessionTest.h"
#include <QByteArray>
#include <QDataStream>
#include <QCoreApplication>
#include "Session.h"
#include "DataManager.h"
#include "data/DataRequirements.h"
#include "data/ServerRequest.h"
#include "data/ServiceDataRequest.h"
#include "data/StreamDataRequest.h"
#include "TestProtocol.h"
#include <iostream>


#include "utility/memCheck.h"

namespace pelican {

CPPUNIT_TEST_SUITE_REGISTRATION( SessionTest );
// class SessionTest 
SessionTest::SessionTest()
    : CppUnit::TestFixture()
{
    int ac = 0;
    _app = new QCoreApplication(ac, NULL);
}

SessionTest::~SessionTest()
{
    delete _app;
}

void SessionTest::setUp()
{
    _out = new QDataStream(&_block,QIODevice::WriteOnly);
    _out->setVersion(QDataStream::Qt_4_0);

    QString id = "1";
    _proto = new TestProtocol(id);
    _data = new DataManager;
    _session = new Session(0, _proto, _data);
}

void SessionTest::tearDown()
{
    delete _data;
    delete _proto;
    delete _session;
    delete _out;
}

void SessionTest::test_processRequest()
{
    {
        // Use Case:
        // Request an acknowledgement
        // Expect the id to be returned
        ServerRequest request(ServerRequest::Acknowledge);

        CPPUNIT_ASSERT_EQUAL( 0, _block.size() );
        _session->processRequest(request, *_out);
        CPPUNIT_ASSERT( _block.size() != 0 );
    }
}

void SessionTest::test_dataReport()
{
}

void SessionTest::test_serviceData()
{
    {
        // Use Case:
        // Request ServiceData that is not supported
        // Expect error message to be returned
        QDataStream out(&_block,QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);

        ServiceDataRequest request;
        _session->processRequest(request, *_out);
    }
}

void SessionTest::test_streamData()
{
    {
        // Use Case:
        // Request StreamData that has no data options
        // Expect error message to be returned
        StreamDataRequest request;
        _session->processRequest(request, *_out);

    }
    {
        // Use Case:
        // Request StreamData for a stream that is not supported by the server
        // Expect error message to be returned
        DataRequirements req;
        StreamDataRequest request;
        request.addDataOption(req);
        _session->processRequest(request, *_out);

    }
    {
        // Use Case:
        // Request StreamData for a stream that is supported but for which no data exists
        // Expect to return with no data
        DataRequirements req;
        StreamDataRequest request;
        request.addDataOption(req);
        _session->processRequest(request, *_out);
    }
    {
        // Use Case:
        // Request StreamData for a stream that is supported and the data exists
        // no service data
        // Expect to return the data
        DataRequirements req;
        StreamDataRequest request;
        request.addDataOption(req);
        _session->processRequest(request, *_out);
    }
    {
        // Use Case:
        // Request StreamData for a stream that is supported and the data exists
        // service data requested, but not available
        // Expect to return no data
        DataRequirements req;
        StreamDataRequest request;
        request.addDataOption(req);
        _session->processRequest(request, *_out);
    }
    {
        // Use Case:
        // Request StreamData for a stream that is supported and the data exists
        // service data requested, and it exists
        // Expect to return data, and service data record
        DataRequirements req;
        StreamDataRequest request;
        request.addDataOption(req);
        _session->processRequest(request, *_out);
    }
}

} // namespace pelican