#ifndef CONFIGTEST_H
#define CONFIGTEST_H

#include <cppunit/extensions/HelperMacros.h>

/**
 * @file ConfigTest.h
 */

/**
 * @class ConfigTest
 *  
 * @brief
 * Class to test the configuration object.
 * 
 * @details
 * QApplication exists for Qt garbage collection from QFile.
 */

class QCoreApplication;

namespace pelican {

class Config;

class ConfigTest : public CppUnit::TestFixture
{
    public:
        CPPUNIT_TEST_SUITE( ConfigTest );
        CPPUNIT_TEST( test_emptyFileName );
        CPPUNIT_TEST( test_fileDoesntExist );
//        CPPUNIT_TEST( test_invalidDocType );
//        CPPUNIT_TEST( test_readConfigFile );
//        CPPUNIT_TEST( test_getModuleConfig() );
//        CPPUNIT_TEST( test_getServerConfig() );
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp();
        void tearDown();

        // Test Methods
        /// Test for an empty configuration file name string.
        void test_emptyFileName();
        /// Test for a configuration file that doesn't exist.
        void test_fileDoesntExist();
        /// Test for an invalid configuration file.
        void test_invalidDocType();

        void test_readConfigFile();
        void test_getModuleConfig();
        void test_getServerConfig();

    public:
        ConfigTest();
        ~ConfigTest();

    private:
        QCoreApplication *app;
};

} // namespace pelican
#endif // CONFIGTEST_H 
