#include <QtGui/QMessageBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QActionGroup>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QTabWidget>
#include <QtGui/QCloseEvent>

#include "pelican/viewer/DataViewer.h"
#include "pelican/viewer/DataBlobWidget.h"
#include "pelican/output/DataBlobClient.h"
#include "pelican/viewer/DataBlobWidgetFactory.h"
#include "pelican/utility/ConfigNode.h"

namespace pelican {

/**
 *@details DataViewer
 *    In development : ideally will use a generic Blob client and be exported to
 *    pelican
 */
DataViewer::DataViewer(const Config& config, const Config::TreeAddress& base, QWidget* parent)
    : QWidget(parent), _client(0)
{
    // setup our factory
    _viewerFactory = new DataBlobWidgetFactory(&config, base);

    // setup generic widgets
    QWidget *bottomFiller = new QWidget;
    bottomFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    _streamTabs = new QTabWidget;

    // setup actions
    QMenuBar* menubar = new QMenuBar;

    // Main Menu
    QMenu* mainMenu = new QMenu(tr("Main"));
    QAction* exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
    mainMenu->addAction(exitAct);
    menubar->addMenu(mainMenu);

    // View Menu
    _viewMenu = new QMenu(tr("View"));
    _viewMenu->setTearOffEnabled(true);
    menubar->addMenu(_viewMenu);
    _streamActionGroup = new QActionGroup(this);
    _streamActionGroup->setExclusive(false);
    _viewMenu->addSeparator();

    // Help Menu
    QMenu* helpMenu = new QMenu(tr("Help"));
    QAction* aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
    helpMenu->addAction(aboutAct);
    menubar->addMenu(helpMenu);

    // set the layout
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(5);
    layout->addWidget(menubar);
    layout->addWidget(_streamTabs);
    //layout->addWidget(bottomFiller);
    setLayout(layout);

    setConfig(config.get(base));
}

/**
 *@details
 */
DataViewer::~DataViewer()
{
    delete _viewerFactory;
    //delete _streamActionGroup;
    //delete _viewMenu;
    //delete _streamTabs;
}

void DataViewer::setStreamViewer( const QString& stream, const QString& viewerType)
{
    _streamViewerMap[stream] = viewerType;
}

DataBlobWidget* DataViewer::getWidgetViewer(const QString& stream) const
{
    DataBlobWidget* widget = (DataBlobWidget*)0;
    try {
        if( _streamViewerMap.contains(stream) ) {
            widget = _viewerFactory->create( _streamViewerMap[stream] );
        }
        else
        {
            widget = _viewerFactory->create( stream );
        }
    }
    catch( const QString& )
    {
        // default is the basic datablob information Widget
        widget = _viewerFactory->create( "DataBlobWidget" );
        //widget = new SubbandSpectrumWidget;
    }
    return widget;
}

void DataViewer::setConfig(const ConfigNode& config)
{
    // set basic connection information
    _port = (quint16)config.getOption("connection", "port").toInt();
    _server = config.getOption("connection", "host");

    // set stream activation defaults
    // read in from the configuration TODO
    // _defaultsEnabled[stream] = true;

}

void DataViewer::setStreamDefault(const QString& stream, bool value)
{
    _defaultsEnabled[stream] = value;
}

/*
 * @details
 * updates the Gui to reflect the specified streams
 */
void DataViewer::_updatedStreams( const QSet<QString>& streams )
{
    // clean up the previous state
    foreach(QAction* action, _streamActionGroup->actions() )
    {
        _streamActionGroup->removeAction(action);
        _viewMenu->removeAction(action);
        delete action;
    }

    // set up the new streams
    foreach(const QString& stream, streams )
    {
        QAction* a = _streamActionGroup->addAction(stream);
        a->setCheckable(true);
        _viewMenu->addAction(a);
        connect( a, SIGNAL(triggered() ), this, SLOT(_streamToggled()) );
        if( _defaultsEnabled.contains(stream) )
        {
            a->setChecked(true);
            enableStream(stream);
        }
        else {
            _defaultsEnabled[stream] = false;
            a->setChecked(false);
        }

    }
}

void DataViewer::setClient(DataBlobClient& client)
{
    _client = &client;
    connect(_client, SIGNAL(dataUpdated(const QString& , DataBlob*)),
                        this, SLOT(dataUpdated(const QString& , DataBlob*)));
}

/**
 * @details
 */
void DataViewer::closeEvent(QCloseEvent* event)
{
    event->accept();
}


void DataViewer::about()
{
    QMessageBox::about(this, tr("Data Stream Viewer"),
            tr("Connects to Pelican data Streams "
                "and displays them."));
}

void DataViewer::connectStreams()
{
    // construct a request for all the streams required
    // TODO
}

void DataViewer::dataUpdated(const QString& stream, DataBlob* blob)
{
    if(  _activeStreams.contains(stream) )
        static_cast<DataBlobWidget*>(_streamTabs->widget(_activeStreams[stream]))->updateData(blob);
}

void DataViewer::enableStream( const QString& stream )
{
    if( ! _activeStreams.contains(stream) )
    {
        _defaultsEnabled[stream] = true;
        _activeStreams[stream] = _streamTabs->addTab( getWidgetViewer(stream), stream );
    }
}

void DataViewer::disableStream( const QString& stream )
{
    if(  _activeStreams.contains(stream) )
    {
        _streamTabs->removeTab( _activeStreams[stream] );
        _activeStreams.remove(stream);
        _defaultsEnabled[stream] = false;
    }
}

bool DataViewer::toggleStream( const QString& stream )
{
    if( _activeStreams.contains(stream) )
    {
        disableStream(stream);
        return false;
    }
    else {
        enableStream(stream);
        return true;
    }
}

void DataViewer::_streamToggled()
{
    QAction* action = static_cast<QAction*>(sender());
    if( action )
        toggleStream( action->text() );
}

} // namespace pelican
