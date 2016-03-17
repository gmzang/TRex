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

#include "astra/FilteredBackProjectionAlgorithm.h"

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <iomanip>
#include <math.h>

#include "astra/AstraObjectManager.h"
#include "astra/ParallelBeamLineKernelProjector2D.h"
#include "astra/Fourier.h"
#include "astra/DataProjector.h"

#include "astra/Logging.h"

using namespace std;

namespace astra {

#include "astra/Projector2DImpl.inl"

// type of the algorithm, needed to register with CAlgorithmFactory
std::string CFilteredBackProjectionAlgorithm::type = "FBP";
const int FFT = 1;
const int IFFT = -1;

//----------------------------------------------------------------------------------------
// Constructor
CFilteredBackProjectionAlgorithm::CFilteredBackProjectionAlgorithm() 
{
	_clear();
}

//----------------------------------------------------------------------------------------
// Destructor
CFilteredBackProjectionAlgorithm::~CFilteredBackProjectionAlgorithm() 
{
	clear();
}

//---------------------------------------------------------------------------------------
// Clear - Constructors
void CFilteredBackProjectionAlgorithm::_clear()
{
	m_pProjector = NULL;
	m_pSinogram = NULL;
	m_pReconstruction = NULL;
	m_bIsInitialized = false;
}

//---------------------------------------------------------------------------------------
// Clear - Public
void CFilteredBackProjectionAlgorithm::clear()
{
	m_pProjector = NULL;
	m_pSinogram = NULL;
	m_pReconstruction = NULL;
	m_bIsInitialized = false;
}


//---------------------------------------------------------------------------------------
// Initialize, use a Config object
bool CFilteredBackProjectionAlgorithm::initialize(const Config& _cfg)
{
	ASTRA_ASSERT(_cfg.self);
	ConfigStackCheck<CAlgorithm> CC("FBPAlgorithm", this, _cfg);
	
	// if already initialized, clear first
	if (m_bIsInitialized) {
		clear();
	}

	// initialization of parent class
	if (!CReconstructionAlgorithm2D::initialize(_cfg)) {
		return false;
	}

	//// projector
	XMLNode node = _cfg.self.getSingleNode("ProjectorId");
	//ASTRA_CONFIG_CHECK(node, "FilteredBackProjection", "No ProjectorId tag specified.");
	//int id = boost::lexical_cast<int>(node.getContent());
	//m_pProjector = CProjector2DManager::getSingleton().get(id);

	//// sinogram data
	//node = _cfg.self.getSingleNode("ProjectionDataId");
	//ASTRA_CONFIG_CHECK(node, "FilteredBackProjection", "No ProjectionDataId tag specified.");
	//id = boost::lexical_cast<int>(node.getContent());
	//m_pSinogram = dynamic_cast<CFloat32ProjectionData2D*>(CData2DManager::getSingleton().get(id));

	//// volume data
	//node = _cfg.self.getSingleNode("ReconstructionDataId");
	//ASTRA_CONFIG_CHECK(node, "FilteredBackProjection", "No ReconstructionDataId tag specified.");
	//id = boost::lexical_cast<int>(node.getContent());
	//m_pReconstruction = dynamic_cast<CFloat32VolumeData2D*>(CData2DManager::getSingleton().get(id));

	node = _cfg.self.getSingleNode("ProjectionIndex");
	if (node) 
	{
		vector<float32> projectionIndex = node.getContentNumericalArray();

		int angleCount = projectionIndex.size();
		int detectorCount = m_pProjector->getProjectionGeometry()->getDetectorCount();

		float32 * sinogramData2D = new float32[angleCount* detectorCount];
		float32 ** sinogramData = new float32*[angleCount];
		for (int i = 0; i < angleCount; i++)
		{
			sinogramData[i] = &(sinogramData2D[i * detectorCount]);
		}

		float32 * projectionAngles = new float32[angleCount];
		float32 detectorWidth = m_pProjector->getProjectionGeometry()->getDetectorWidth();

		for (int i = 0; i < angleCount; i ++) {
			if (projectionIndex[i] > m_pProjector->getProjectionGeometry()->getProjectionAngleCount() -1 )
			{
				ASTRA_ERROR("Invalid Projection Index");
				return false;
			} else {
				int orgIndex = (int)projectionIndex[i];

				for (int iDetector=0; iDetector < detectorCount; iDetector++) 
				{
					sinogramData2D[i*detectorCount+ iDetector] = m_pSinogram->getData2D()[orgIndex][iDetector];
				}
//				sinogramData[i] = m_pSinogram->getSingleProjectionData(projectionIndex[i]);
				projectionAngles[i] = m_pProjector->getProjectionGeometry()->getProjectionAngle((int)projectionIndex[i] );

			}
		}

		CParallelProjectionGeometry2D * pg = new CParallelProjectionGeometry2D(angleCount, detectorCount,detectorWidth,projectionAngles);
		m_pProjector = new CParallelBeamLineKernelProjector2D(pg,m_pReconstruction->getGeometry());
		m_pSinogram = new CFloat32ProjectionData2D(pg, sinogramData2D);
	}

	// TODO: check that the angles are linearly spaced between 0 and pi

	// success
	m_bIsInitialized = _check();
	return m_bIsInitialized;
}

//---------------------------------------------------------------------------------------
// Get Information - all
map<string,boost::any> CFilteredBackProjectionAlgorithm::getInformation() 
{
	map<string, boost::any> result;
	result["ProjectorId"] = getInformation("ProjectorId");
	result["ProjectionDataId"] = getInformation("ProjectionDataId");
	result["VolumeDataId"] = getInformation("VolumeDataId");
	return mergeMap<string,boost::any>(CAlgorithm::getInformation(), result);
};

//---------------------------------------------------------------------------------------
// Get Information - specific
boost::any CFilteredBackProjectionAlgorithm::getInformation(std::string _sIdentifier) 
{
	if (_sIdentifier == "ProjectorId") {
		int iIndex = CProjector2DManager::getSingleton().getIndex(m_pProjector);
		if (iIndex != 0) return iIndex;
		return std::string("not in manager");
	} else if (_sIdentifier == "ProjectionDataId") {
		int iIndex = CData2DManager::getSingleton().getIndex(m_pSinogram);
		if (iIndex != 0) return iIndex;
		return std::string("not in manager");
	} else if (_sIdentifier == "VolumeDataId") {
		int iIndex = CData2DManager::getSingleton().getIndex(m_pReconstruction);
		if (iIndex != 0) return iIndex;
		return std::string("not in manager");
	}
	return CAlgorithm::getInformation(_sIdentifier);
};

//----------------------------------------------------------------------------------------
// Initialize
bool CFilteredBackProjectionAlgorithm::initialize(CProjector2D* _pProjector, 
												  CFloat32VolumeData2D* _pVolume,
												  CFloat32ProjectionData2D* _pSinogram)
{
	// store classes
	m_pProjector = _pProjector;
	m_pReconstruction = _pVolume;
	m_pSinogram = _pSinogram;


	// TODO: check that the angles are linearly spaced between 0 and pi

	// success
	m_bIsInitialized = _check();
	return m_bIsInitialized;
}

//----------------------------------------------------------------------------------------
// Check
bool CFilteredBackProjectionAlgorithm::_check() 
{
	ASTRA_CONFIG_CHECK(CReconstructionAlgorithm2D::_check(), "FBP", "Error in ReconstructionAlgorithm2D initialization");

	// success
	return true;
}


//----------------------------------------------------------------------------------------
// Iterate
void CFilteredBackProjectionAlgorithm::run(int _iNrIterations)
{
	ASTRA_ASSERT(m_bIsInitialized);

	// start timer
	m_ulTimer = CPlatformDepSystemCode::getMSCount();

	// Filter sinogram
	CFloat32ProjectionData2D filteredSinogram(m_pSinogram->getGeometry(), m_pSinogram->getData());
	performFiltering(&filteredSinogram);

	// Back project
	m_pReconstruction->setData(0.0f);
	projectData(m_pProjector,
	            DefaultBPPolicy(m_pReconstruction, &filteredSinogram));

	// Scale data
	int iAngleCount = m_pProjector->getProjectionGeometry()->getProjectionAngleCount();
	(*m_pReconstruction) *= (PI/2)/iAngleCount;

	// Check limits.
	if (m_bUseMinConstraint)
		m_pReconstruction->clampMin(m_fMinValue);
	if (m_bUseMaxConstraint)
		m_pReconstruction->clampMax(m_fMaxValue);

	// end timer
	m_ulTimer = CPlatformDepSystemCode::getMSCount() - m_ulTimer;

	// Compute metrics.
	computeIterationMetrics(0, 1);

	m_pReconstruction->updateStatistics();
}


//----------------------------------------------------------------------------------------
void CFilteredBackProjectionAlgorithm::performFiltering(CFloat32ProjectionData2D * _pFilteredSinogram)
{
	ASTRA_ASSERT(_pFilteredSinogram != NULL);
	ASTRA_ASSERT(_pFilteredSinogram->getAngleCount() == m_pSinogram->getAngleCount());
	ASTRA_ASSERT(_pFilteredSinogram->getDetectorCount() == m_pSinogram->getDetectorCount());


	int iAngleCount = m_pProjector->getProjectionGeometry()->getProjectionAngleCount();
	int iDetectorCount = m_pProjector->getProjectionGeometry()->getDetectorCount();


	// We'll zero-pad to the smallest power of two at least 64 and
	// at least 2*iDetectorCount
	int zpDetector = 64;
	int nextPow2 = 5;
	while (zpDetector < iDetectorCount*2) {
		zpDetector *= 2;
		nextPow2++;
	}

	// Create filter
	float32* filter = new float32[zpDetector];

	for (int iDetector = 0; iDetector <= zpDetector/2; iDetector++)
		filter[iDetector] = (2.0f * iDetector)/zpDetector;

	for (int iDetector = zpDetector/2+1; iDetector < zpDetector; iDetector++)
		filter[iDetector] = (2.0f * (zpDetector - iDetector)) / zpDetector;


	float32* pfRe = new float32[iAngleCount * zpDetector];
	float32* pfIm = new float32[iAngleCount * zpDetector];

	// Copy and zero-pad data
	for (int iAngle = 0; iAngle < iAngleCount; ++iAngle) {
		float32* pfReRow = pfRe + iAngle * zpDetector;
		float32* pfImRow = pfIm + iAngle * zpDetector;
		float32* pfDataRow = _pFilteredSinogram->getData() + iAngle * iDetectorCount;
		for (int iDetector = 0; iDetector < iDetectorCount; ++iDetector) {
			pfReRow[iDetector] = pfDataRow[iDetector];
			pfImRow[iDetector] = 0.0f;
		}
		for (int iDetector = iDetectorCount; iDetector < zpDetector; ++iDetector) {
			pfReRow[iDetector] = 0.0f;
			pfImRow[iDetector] = 0.0f;
		}
	}

	// in-place FFT
	for (int iAngle = 0; iAngle < iAngleCount; ++iAngle) {
		float32* pfReRow = pfRe + iAngle * zpDetector;
		float32* pfImRow = pfIm + iAngle * zpDetector;

		fastTwoPowerFourierTransform1D(zpDetector, pfReRow, pfImRow, pfReRow, pfImRow, 1, 1, false);
	}

	// Filter
	for (int iAngle = 0; iAngle < iAngleCount; ++iAngle) {
		float32* pfReRow = pfRe + iAngle * zpDetector;
		float32* pfImRow = pfIm + iAngle * zpDetector;
		for (int iDetector = 0; iDetector < zpDetector; ++iDetector) {
			pfReRow[iDetector] *= filter[iDetector];
			pfImRow[iDetector] *= filter[iDetector];
		}
	}

	// in-place inverse FFT
	for (int iAngle = 0; iAngle < iAngleCount; ++iAngle) {
		float32* pfReRow = pfRe + iAngle * zpDetector;
		float32* pfImRow = pfIm + iAngle * zpDetector;

		fastTwoPowerFourierTransform1D(zpDetector, pfReRow, pfImRow, pfReRow, pfImRow, 1, 1, true);
	}

	// Copy data back
	for (int iAngle = 0; iAngle < iAngleCount; ++iAngle) {
		float32* pfReRow = pfRe + iAngle * zpDetector;
		float32* pfDataRow = _pFilteredSinogram->getData() + iAngle * iDetectorCount;
		for (int iDetector = 0; iDetector < iDetectorCount; ++iDetector)
			pfDataRow[iDetector] = pfReRow[iDetector];
	}

	delete[] pfRe;
	delete[] pfIm;
	delete[] filter;
}

}
