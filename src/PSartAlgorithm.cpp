/*
-----------------------------------------------------------------------
Copyright: 2010-2015, iMinds-Vision Lab, University of Antwerp
           2014-2015, CWI, Amsterdam

Contact: astra@uantwerpen.be
Website: http://sf.net/projects/astra-toolbox

This file is part of the ASTRA Toolbox.


The ASTRA Toolbox is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The ASTRA Toolbox is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the ASTRA Toolbox. If not, see <http://www.gnu.org/licenses/>.

-----------------------------------------------------------------------
$Id$
*/

#include "astra/PSartAlgorithm.h"

#include <boost/lexical_cast.hpp>

#include "astra/AstraObjectManager.h"
#include "astra/DataProjectorPolicies.h"

using namespace std;

namespace astra {

#include "astra/Projector2DImpl.inl"

// type of the algorithm, needed to register with CAlgorithmFactory
std::string CPSartAlgorithm::type = "PSART";


//---------------------------------------------------------------------------------------
// Clear - Constructors
void CPSartAlgorithm::_clear()
{
	CReconstructionAlgorithm2D::_clear();
	m_piProjectionOrder = NULL;
	m_iProjectionCount = 0;
	m_iCurrentProjection = 0;
	m_bIsInitialized = false;
	m_iIterationCount = 0;
}

//---------------------------------------------------------------------------------------
// Clear - Public
void CPSartAlgorithm::clear()
{
	CReconstructionAlgorithm2D::clear();
	if (m_piProjectionOrder) {
		delete[] m_piProjectionOrder;
		m_piProjectionOrder = NULL;
	}
	m_iProjectionCount = 0;
	m_iCurrentProjection = 0;
	m_bIsInitialized = false;
	m_iIterationCount = 0;
}

//----------------------------------------------------------------------------------------
// Constructor
CPSartAlgorithm::CPSartAlgorithm() 
{
	_clear();
}

//----------------------------------------------------------------------------------------
// Constructor
CPSartAlgorithm::CPSartAlgorithm(CProjector2D* _pProjector, 
							   CFloat32ProjectionData2D* _pSinogram, 
							   CFloat32VolumeData2D* _pReconstruction) 
{
	_clear();
	initialize(_pProjector, _pSinogram, _pReconstruction);
}

//----------------------------------------------------------------------------------------
// Constructor
CPSartAlgorithm::CPSartAlgorithm(CProjector2D* _pProjector, 
							   CFloat32ProjectionData2D* _pSinogram, 
							   CFloat32VolumeData2D* _pReconstruction,
							   int* _piProjectionOrder, 
							   int _iProjectionCount)
{
	_clear();
	initialize(_pProjector, _pSinogram, _pReconstruction, _piProjectionOrder, _iProjectionCount);
}

//----------------------------------------------------------------------------------------
// Destructor
CPSartAlgorithm::~CPSartAlgorithm() 
{
	clear();
}

//---------------------------------------------------------------------------------------
// Initialize - Config
bool CPSartAlgorithm::initialize(const Config& _cfg)
{
	assert(_cfg.self);
	ConfigStackCheck<CAlgorithm> CC("PSartAlgorithm", this, _cfg);
	
	// if already initialized, clear first
	if (m_bIsInitialized) {
		clear();
	}

	// initialization of parent class
	if (!CReconstructionAlgorithm2D::initialize(_cfg)) {
		return false;
	}

	// projection order
	m_iCurrentProjection = 0;
	m_iProjectionCount = m_pProjector->getProjectionGeometry()->getProjectionAngleCount();
	string projOrder = _cfg.self.getOption("ProjectionOrder", "sequential");
	CC.markOptionParsed("ProjectionOrder");
	if (projOrder == "sequential") {
		m_piProjectionOrder = new int[m_iProjectionCount];
		for (int i = 0; i < m_iProjectionCount; i++) {
			m_piProjectionOrder[i] = i;
		}
	} else if (projOrder == "random") {
		m_piProjectionOrder = new int[m_iProjectionCount];
		for (int i = 0; i < m_iProjectionCount; i++) {
			m_piProjectionOrder[i] = i;
		}
		for (int i = 0; i < m_iProjectionCount-1; i++) {
			int k = (rand() % (m_iProjectionCount - i));
			int t = m_piProjectionOrder[i];
			m_piProjectionOrder[i] = m_piProjectionOrder[i + k];
			m_piProjectionOrder[i + k] = t;
		}
	} else if (projOrder == "custom") {
		vector<float32> projOrderList = _cfg.self.getOptionNumericalArray("ProjectionOrderList");
		m_piProjectionOrder = new int[projOrderList.size()];
		for (int i = 0; i < m_iProjectionCount; i++) {
			m_piProjectionOrder[i] = static_cast<int>(projOrderList[i]);
		}
		CC.markOptionParsed("ProjectionOrderList");
	}

	// Lambda and Alpha
	m_fAlpha = _cfg.self.getOptionNumerical("alpha", 1.0f);
	m_fLambda = _cfg.self.getOptionNumerical("lambda", 1.0f);
	CC.markOptionParsed("alpha");
	CC.markOptionParsed("lambda");

	// Input volume
	XMLNode node = _cfg.self.getSingleNode("ProxInputDataId");
	ASTRA_CONFIG_CHECK(node, "PSART", "No Proximal Input tag specified.");
	int id = boost::lexical_cast<int>(node.getContent());
	m_pProxInput = dynamic_cast<CFloat32VolumeData2D*>(CData2DManager::getSingleton().get(id));
	CC.markNodeParsed("ProxInputDataId");

	// create data objects
	m_pTotalRayLength = new CFloat32ProjectionData2D(m_pProjector->getProjectionGeometry());
	m_pTotalPixelWeight = new CFloat32VolumeData2D(m_pProjector->getVolumeGeometry());
	m_pDiffSinogram = new CFloat32ProjectionData2D(m_pProjector->getProjectionGeometry());

	// success
	m_bIsInitialized = _check();
	return m_bIsInitialized;
}

//---------------------------------------------------------------------------------------
// Initialize - C++
bool CPSartAlgorithm::initialize(CProjector2D* _pProjector, 
								CFloat32ProjectionData2D* _pSinogram, 
								CFloat32VolumeData2D* _pReconstruction)
{
	// if already initialized, clear first
	if (m_bIsInitialized) {
		clear();
	}

	// required classes
	m_pProjector = _pProjector;
	m_pSinogram = _pSinogram;
	m_pReconstruction = _pReconstruction;

	// ray order
	m_iCurrentProjection = 0;
	m_iProjectionCount = _pProjector->getProjectionGeometry()->getProjectionAngleCount();
	m_piProjectionOrder = new int[m_iProjectionCount];
	for (int i = 0; i < m_iProjectionCount; i++) {
		m_piProjectionOrder[i] = i;
	}

	// create data objects
	m_pTotalRayLength = new CFloat32ProjectionData2D(m_pProjector->getProjectionGeometry());
	m_pTotalPixelWeight = new CFloat32VolumeData2D(m_pProjector->getVolumeGeometry());
	m_pDiffSinogram = new CFloat32ProjectionData2D(m_pProjector->getProjectionGeometry());

	// success
	m_bIsInitialized = _check();
	return m_bIsInitialized;
}

//---------------------------------------------------------------------------------------
// Initialize - C++
bool CPSartAlgorithm::initialize(CProjector2D* _pProjector, 
								CFloat32ProjectionData2D* _pSinogram, 
								CFloat32VolumeData2D* _pReconstruction,
								int* _piProjectionOrder, 
								int _iProjectionCount)
{
	// required classes
	m_pProjector = _pProjector;
	m_pSinogram = _pSinogram;
	m_pReconstruction = _pReconstruction;

	// ray order
	m_iCurrentProjection = 0;
	m_iProjectionCount = _iProjectionCount;
	m_piProjectionOrder = new int[m_iProjectionCount];
	for (int i = 0; i < m_iProjectionCount; i++) {
		m_piProjectionOrder[i] = _piProjectionOrder[i];
	}

	// create data objects
	m_pTotalRayLength = new CFloat32ProjectionData2D(m_pProjector->getProjectionGeometry());
	m_pTotalPixelWeight = new CFloat32VolumeData2D(m_pProjector->getVolumeGeometry());
	m_pDiffSinogram = new CFloat32ProjectionData2D(m_pProjector->getProjectionGeometry());

	// success
	m_bIsInitialized = _check();
	return m_bIsInitialized;
}

//----------------------------------------------------------------------------------------
bool CPSartAlgorithm::_check()
{
	// check base class
	ASTRA_CONFIG_CHECK(CReconstructionAlgorithm2D::_check(), "PSART", "Error in ReconstructionAlgorithm2D initialization");

	// check projection order all within range
	for (int i = 0; i < m_iProjectionCount; ++i) {
		ASTRA_CONFIG_CHECK(0 <= m_piProjectionOrder[i] && m_piProjectionOrder[i] < m_pProjector->getProjectionGeometry()->getProjectionAngleCount(), "PSART", "Projection Order out of range.");
	}

	return true;
}

//---------------------------------------------------------------------------------------
// Information - All
map<string,boost::any> CPSartAlgorithm::getInformation() 
{
	map<string, boost::any> res;
	res["ProjectionOrder"] = getInformation("ProjectionOrder");
	return mergeMap<string,boost::any>(CReconstructionAlgorithm2D::getInformation(), res);
};

//---------------------------------------------------------------------------------------
// Information - Specific
boost::any CPSartAlgorithm::getInformation(std::string _sIdentifier) 
{
	if (_sIdentifier == "ProjectionOrder") {
		vector<float32> res;
		for (int i = 0; i < m_iProjectionCount; i++) {
			res.push_back(m_piProjectionOrder[i]);
		}
		return res;
	}
	return CAlgorithm::getInformation(_sIdentifier);
};

//----------------------------------------------------------------------------------------
// Iterate
void CPSartAlgorithm::run(int _iNrIterations)
{
	// check initialized
	ASTRA_ASSERT(m_bIsInitialized);

	m_bShouldAbort = false;

	int iIteration = 0;

	// data projectors
	CDataProjectorInterface* pForwardProjector;
	CDataProjectorInterface* pBackProjector;

	m_pTotalRayLength->setData(0.0f);
	m_pTotalPixelWeight->setData(0.0f);

	// backprojection data projector
	pBackProjector = dispatchDataProjector(
			m_pProjector, 
			SinogramMaskPolicy(m_pSinogramMask),														// sinogram mask
			ReconstructionMaskPolicy(m_pReconstructionMask),											// reconstruction mask
			SIRTBPPolicy(m_pReconstruction, m_pDiffSinogram, 
						 m_pTotalPixelWeight, m_pTotalRayLength, m_fAlpha),	// SIRT backprojection
			m_bUseSinogramMask, m_bUseReconstructionMask, true // options on/off
		); 

	// first time forward projection data projector,
	// also computes total pixel weight and total ray length
	pForwardProjector = dispatchDataProjector(
			m_pProjector, 
			SinogramMaskPolicy(m_pSinogramMask),														// sinogram mask
			ReconstructionMaskPolicy(m_pReconstructionMask),											// reconstruction mask
			Combine3Policy<DiffFPPolicy, TotalPixelWeightPolicy, TotalRayLengthPolicy>(					// 3 basic operations
				DiffFPPolicy(m_pReconstruction, m_pDiffSinogram, m_pSinogram),								// forward projection with difference calculation
				TotalPixelWeightPolicy(m_pTotalPixelWeight),												// calculate the total pixel weights
				TotalRayLengthPolicy(m_pTotalRayLength)),													// calculate the total ray lengths
			m_bUseSinogramMask, m_bUseReconstructionMask, true											 // options on/off
		);



	// iteration loop
	for (; iIteration < _iNrIterations && !m_bShouldAbort; ++iIteration) {

		int iProjection = m_piProjectionOrder[m_iIterationCount % m_iProjectionCount];
	
		// forward projection and difference calculation
		m_pTotalPixelWeight->setData(0.0f);
		pForwardProjector->projectSingleProjection(iProjection);
		// backprojection
		pBackProjector->projectSingleProjection(iProjection);
		// update iteration count
		m_iIterationCount++;

		if (m_bUseMinConstraint)
			m_pReconstruction->clampMin(m_fMinValue);
		if (m_bUseMaxConstraint)
			m_pReconstruction->clampMax(m_fMaxValue);
	}


	ASTRA_DELETE(pForwardProjector);
	ASTRA_DELETE(pBackProjector);














	//// check initialized
 // 	ASTRA_ASSERT(m_bIsInitialized);

	//// variables
	//int iIteration, iDetector;
	//int baseIndex, iPixel;
	//float32* pfGamma = new float32[m_pReconstruction->getSize()];
	//float32* pfBeta = new float32[m_pProjector->getProjectionGeometry()->getDetectorCount()];
	//float32* pfProjectionDiff = new float32[m_pProjector->getProjectionGeometry()->getDetectorCount()];

	//// ITERATE
	//for (iIteration = _iNrIterations-1; iIteration >= 0; --iIteration) {
	//
	//	// reset gamma	
	//	memset(pfGamma, 0, sizeof(float32) * m_pReconstruction->getSize());
	//	memset(pfBeta, 0, sizeof(float32) * m_pProjector->getProjectionGeometry()->getDetectorCount());
	//
	//	// get current projection angle
	//	int iProjection = m_piProjectionOrder[m_iCurrentProjection];
	//	m_iCurrentProjection = (m_iCurrentProjection + 1) % m_iProjectionCount;
	//	int iProjectionWeightCount = m_pProjector->getProjectionWeightsCount(iProjection);
	//
	//	// allocate memory for the pixel buffer
	//	SPixelWeight* pPixels = new SPixelWeight[m_pProjector->getProjectionWeightsCount(iProjection) * m_pProjector->getProjectionGeometry()->getDetectorCount()];
	//	int* piRayStoredPixelCount = new int[m_pProjector->getProjectionGeometry()->getDetectorCount()];

	//	// compute weights for this projection
	//	m_pProjector->computeProjectionRayWeights(iProjection, pPixels, piRayStoredPixelCount);
	//
	//	// calculate projection difference in each detector
	//	for (iDetector = m_pProjector->getProjectionGeometry()->getDetectorCount()-1; iDetector >= 0; --iDetector) {

	//		if (m_bUseSinogramMask && m_pSinogramMask->getData2D()[iProjection][iDetector] == 0) continue;	

	//		// index base of the pixel in question
	//		baseIndex = iDetector * iProjectionWeightCount;
	//
	//		// set the initial projection difference to the sinogram value
	//		pfProjectionDiff[iDetector] = m_pSinogram->getData2DConst()[iProjection][iDetector];
	//
	//		// update projection difference, beta and gamma
	//		for (iPixel = piRayStoredPixelCount[iDetector]-1; iPixel >= 0; --iPixel) {

	//			// pixel must be loose
	//			if (m_bUseReconstructionMask && m_pReconstructionMask->getData()[pPixels[baseIndex+iPixel].m_iIndex] == 0) continue;

	//			// subtract projection value from projection difference 
	//			pfProjectionDiff[iDetector] -= 
	//				pPixels[baseIndex+iPixel].m_fWeight * m_pReconstruction->getDataConst()[pPixels[baseIndex+iPixel].m_iIndex];
	//				
	//			// update beta and gamma if this pixel lies inside a loose part
	//			pfBeta[iDetector] += pPixels[baseIndex+iPixel].m_fWeight;
	//			pfGamma[pPixels[baseIndex+iPixel].m_iIndex] += pPixels[baseIndex+iPixel].m_fWeight;
	//		}
	//
	//	}
	//
	//	// back projection
	//	for (iDetector = m_pProjector->getProjectionGeometry()->getDetectorCount()-1; iDetector >= 0; --iDetector) {
	//		
	//		if (m_bUseSinogramMask && m_pSinogramMask->getData2D()[iProjection][iDetector] == 0) continue;	

	//		// index base of the pixel in question
	//		baseIndex = iDetector * iProjectionWeightCount;

	//		// update pixel values
	//		for (iPixel = piRayStoredPixelCount[iDetector]-1; iPixel >= 0; --iPixel) {

	//
	//			// pixel must be loose
	//			if (m_bUseReconstructionMask && m_pReconstructionMask->getData()[pPixels[baseIndex+iPixel].m_iIndex] == 0) continue;

	//			

	//			// update reconstruction volume
	//			float32 fGammaBeta = pfGamma[pPixels[baseIndex+iPixel].m_iIndex] * pfBeta[iDetector];
	//			if ((fGammaBeta > 0.01f) || (fGammaBeta < -0.01f)) {	
	//				m_pReconstruction->getData()[pPixels[baseIndex+iPixel].m_iIndex] += 
	//					pPixels[baseIndex+iPixel].m_fWeight * pfProjectionDiff[iDetector] / fGammaBeta;
	//			}

	//			// constraints
	//			if (m_bUseMinConstraint && m_pReconstruction->getData()[pPixels[baseIndex+iPixel].m_iIndex] < m_fMinValue) {
	//				m_pReconstruction->getData()[pPixels[baseIndex+iPixel].m_iIndex] = m_fMinValue;
	//			}
	//			if (m_bUseMaxConstraint && m_pReconstruction->getData()[pPixels[baseIndex+iPixel].m_iIndex] > m_fMaxValue) {
	//				m_pReconstruction->getData()[pPixels[baseIndex+iPixel].m_iIndex] = m_fMaxValue;
	//			}
	//		}
	//	}
	//
	//	// garbage disposal
	//	delete[] pPixels;
	//	delete[] piRayStoredPixelCount;
	//}

	//// garbage disposal
	//delete[] pfGamma;
	//delete[] pfBeta;
	//delete[] pfProjectionDiff;

	//// update statistics
	//m_pReconstruction->updateStatistics();
}
//----------------------------------------------------------------------------------------

} // namespace astra
