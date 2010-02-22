#ifndef ZENITHIMAGERDFT_H
#define ZENITHIMAGERDFT_H

#include "AbstractModule.h"
#include "data/ImageData.h"
#include "data/VisibilityData.h"
#include "data/AntennaPositions.h"
#include "data/FrequencyList.h"
#include "utility/ConfigNode.h"


/**
 * @file ZenithImagerDft.h
 */

namespace pelican {

/**
 * @class ZenithImagerDft
 *
 * @brief
 * Provides basic imaging routines for whole-sky views.
 *
 * @details
 */
class ZenithImagerDft : public AbstractModule
{
    private:
        friend class ZenithImagerDftTest;

    public:
        enum { POL_X, POL_Y, POL_BOTH };

    public:
        /// Module constructor.
        ZenithImagerDft(const ConfigNode& config);

        /// Module destructor.
        ~ZenithImagerDft();

        /// Sets the channels (\p c) produce an image from.
        /// Method overrides the selected channels from the configuration
        void setChannels(const std::vector<unsigned>& c) { _channels = c; }

        /// Sets the polarisation selection.
        /// Method overrides the selected channels from the configuration
        void setPolarisations(const unsigned& p) { _polarisation = p; }

        /// Sets the image size
        void setSize(const unsigned& sizeL, const unsigned& sizeM);

        /// Sets the image cellsize
        void setCellsize(const double& cellsizeL, const double& cellsizeM);

        /// Sets the image size and cellsize
        void setDimensions(const unsigned& sizeL, const unsigned& sizeM,
                const double& cellsizeL, const double& cellsizeM);

        /// Set the image size to the full sky.
        void setFullSky();

        /// Runs the module.
        void run(QHash<QString, DataBlob*>& data);

    private:
        /// Extract the configuration from the XML node setting default where required.
        void _getConfiguration(const ConfigNode& config);

        /// Generates an array of image coordinates in radians.
        void _calculateImageCoords(const double& cellsize, const unsigned& nPixels,
                real_t* coords);

        /// Fetches data blob pointers from the blob hash.
        void _fetchDataBlobs(QHash<QString, DataBlob*>& data);

        /// Calculate DFT weights for imaging.
        void _calculateWeights(const unsigned& nAnt, real_t* antPos,
                const double& frequency, const unsigned& nCoords,
                real_t* imageCoord, complex_t* weights,
                const double& sign = 1.0);

        /// Construct the image by DFT.
        void _makeImageDft(const unsigned& nAnt, real_t* antPosX,
                real_t* antPosY, complex_t* vis, const double& frequency,
                const unsigned& nL, const unsigned& nM, real_t* coordL,
                real_t* coordM, real_t *image);

        /// Element wise multiplication of two complex weights vectors
        void _multWeights(const unsigned& nAnt, complex_t* weightsXL,
                complex_t *weightsYM, complex_t *weights);

        /// Matrix vector multiply of visibilities by a weights vector
        void _multMatrixVector(const unsigned& nAnt, complex_t* visMatrix,
                complex_t *weights, complex_t *result);

        /// Vector-vector dot product taking the complex conjugate of the
        /// second vector.
        complex_t _vectorDotConj(const unsigned& n, complex_t* a, complex_t* b);

        /// Cut the image outside unit radius in l, m.
        void _cutHemisphere(real_t* image, unsigned& nL, unsigned& nM,
                real_t *l, real_t *m);

        /// Sets the cellsize corresponding to a full sky image
        void _setCellsizeFullSky();

    private:
          VisibilityData *_vis;               ///< Visibility amplitude matrix.
          AntennaPositions *_antPos;          ///< Visibility positions matrix.
          ImageData *_image;                  ///< Image amplitude matrix.

          std::vector<unsigned> _channels;    ///< Selected channel list.
          unsigned _nChannels;
          double _maxFrequency;
          unsigned _polarisation;             ///< Selection polarisation (enumeration).
          bool _fullSky;                      ///< Image the full sky (overrides cellsize)
          unsigned _sizeL;                    ///< Image size in l (x) pixels.
          unsigned _sizeM;                    ///< Image size in m (y) pixels.
          double _cellsizeL;                  ///< Image pixel increment in m (y) direction.
          double _cellsizeM;                  ///< Image pixel increment in l (x) direction.

          std::vector<real_t> _coordL;        ///< Image l (x) coordinates in radians.
          std::vector<real_t> _coordM;        ///< Image m (y) coordinates in radians.

          std::vector<complex_t> _weightsXL;  ///< DFT weights for XL coordinates
          std::vector<complex_t> _weightsYM;  ///< DFT weights for YM coordinates
          std::vector<complex_t> _weights;    ///< Vector of DFT weights for a single l,m
          std::vector<complex_t> _temp;       ///< Temporary vector used in the DFT.
};

} // namespace pelican

#endif // ZENITHIMAGERDFT_H
