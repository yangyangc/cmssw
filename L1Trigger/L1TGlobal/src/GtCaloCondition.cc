/**
 * \class GtCaloCondition
 *
 *
 * Description: evaluation of a CondCalo condition.
 *
 * Implementation:
 *    <TODO: enter implementation details>
 *
 *
 */

// this class header
#include "L1Trigger/L1TGlobal/interface/GtCaloCondition.h"

// system include files
#include <iostream>
#include <iomanip>

#include <string>
#include <vector>
#include <algorithm>

// user include files
//   base classes
#include "CondFormats/L1TObjects/interface/GlobalCaloTemplate.h"
#include "L1Trigger/L1TGlobal/interface/ConditionEvaluation.h"

#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerReadoutSetupFwd.h"

#include "DataFormats/L1Trigger/interface/L1Candidate.h"
/*#include "DataFormats/L1GlobalCaloTrigger/interface/L1GctCand.h"
#include "DataFormats/L1GlobalCaloTrigger/interface/L1GctEmCand.h"
#include "DataFormats/L1GlobalCaloTrigger/interface/L1GctJetCand.h"
*/

#include "CondFormats/L1TObjects/interface/GlobalStableParameters.h"
#include "CondFormats/DataRecord/interface/L1TGlobalStableParametersRcd.h"

#include "L1Trigger/GlobalTrigger/interface/L1GlobalTriggerFunctions.h"
#include "L1Trigger/L1TGlobal/interface/GtBoard.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/MessageLogger/interface/MessageDrop.h"

// constructors
//     default
l1t::GtCaloCondition::GtCaloCondition() :
    ConditionEvaluation() {

    m_ifCaloEtaNumberBits = -1;
    m_corrParDeltaPhiNrBins = 0;

}

//     from base template condition (from event setup usually)
l1t::GtCaloCondition::GtCaloCondition(const GlobalCondition* caloTemplate, const GtBoard* ptrGTB,
        const int nrL1EG,
        const int nrL1Jet,
        const int nrL1Tau,
        const int ifCaloEtaNumberBits) :
    ConditionEvaluation(),
    m_gtCaloTemplate(static_cast<const GlobalCaloTemplate*>(caloTemplate)),
    m_uGtB(ptrGTB),
    m_ifCaloEtaNumberBits(ifCaloEtaNumberBits)
{

    m_corrParDeltaPhiNrBins = 0;

    // maximum number of  objects received for the evaluation of the condition
    // retrieved before from event setup
    // for a CondCalo, all objects ar of same type, hence it is enough to get the
    // type for the first object

    switch ((m_gtCaloTemplate->objectType())[0]) {
        case NoIsoEG:
            m_condMaxNumberObjects = nrL1EG;
            break;
/*        case IsoEG:
            m_condMaxNumberObjects = nrL1IsoEG;
            break;
*/
        case CenJet:
            m_condMaxNumberObjects = nrL1Jet;
            break;
/*        case ForJet:
            m_condMaxNumberObjects = nrL1ForJet;
            break;
*/
        case TauJet:
            m_condMaxNumberObjects = nrL1Tau;
            break;
        default:
            m_condMaxNumberObjects = 0;
            break;
    }

}

// copy constructor
void l1t::GtCaloCondition::copy(const l1t::GtCaloCondition& cp) {

    m_gtCaloTemplate = cp.gtCaloTemplate();
    m_uGtB = cp.getuGtB();

    m_ifCaloEtaNumberBits = cp.gtIfCaloEtaNumberBits();
    m_corrParDeltaPhiNrBins = cp.m_corrParDeltaPhiNrBins;

    m_condMaxNumberObjects = cp.condMaxNumberObjects();
    m_condLastResult = cp.condLastResult();
    m_combinationsInCond = cp.getCombinationsInCond();

    m_verbosity = cp.m_verbosity;

}

l1t::GtCaloCondition::GtCaloCondition(const l1t::GtCaloCondition& cp) :
    ConditionEvaluation() {

    copy(cp);

}

// destructor
l1t::GtCaloCondition::~GtCaloCondition() {

    // empty

}

// equal operator
l1t::GtCaloCondition& l1t::GtCaloCondition::operator=(const l1t::GtCaloCondition& cp) {
    copy(cp);
    return *this;
}

// methods
void l1t::GtCaloCondition::setGtCaloTemplate(const GlobalCaloTemplate* caloTempl) {

    m_gtCaloTemplate = caloTempl;

}

///   set the pointer to uGT GtBoard
void l1t::GtCaloCondition::setuGtB(const GtBoard* ptrGTB) {

    m_uGtB = ptrGTB;

}

//   set the number of bits for eta of calorimeter objects
void l1t::GtCaloCondition::setGtIfCaloEtaNumberBits(const int& ifCaloEtaNumberBitsValue) {

    m_ifCaloEtaNumberBits = ifCaloEtaNumberBitsValue;

}

//   set the maximum number of bins for the delta phi scales
void l1t::GtCaloCondition::setGtCorrParDeltaPhiNrBins(
        const int& corrParDeltaPhiNrBins) {

    m_corrParDeltaPhiNrBins = corrParDeltaPhiNrBins;

}

// try all object permutations and check spatial correlations, if required
const bool l1t::GtCaloCondition::evaluateCondition(const int bxEval) const {

    // number of trigger objects in the condition
    int nObjInCond = m_gtCaloTemplate->nrObjects();
    //LogTrace("L1GlobalTrigger") << "  nObjInCond: " << nObjInCond
    //    << std::endl;

    // the candidates

    // objectType() gives the type for nrObjects() only,
    // but in a CondCalo all objects have the same type
    // take type from the type of the first object

    const BXVector<const l1t::L1Candidate*>* candVec;

    switch ((m_gtCaloTemplate->objectType())[0]) {
        case NoIsoEG:
            candVec = m_uGtB->getCandL1EG();
            break;

        case CenJet:
            candVec = m_uGtB->getCandL1Jet();
            break;

        case TauJet:
            candVec = m_uGtB->getCandL1Tau();
            break;

        default:
            return false;
            break;
    }

    // Look at objects in bx = bx + relativeBx
    int useBx = bxEval + m_gtCaloTemplate->condRelativeBx();

    // Fail condition if attempting to get Bx outside of range
    if( ( useBx < candVec->getFirstBX() ) ||
	( useBx > candVec->getLastBX() ) ) {
      return false;
    }


    int numberObjects = candVec->size(useBx);
    //LogTrace("L1GlobalTrigger") << "  numberObjects: " << numberObjects
    //    << std::endl;
    if (numberObjects < nObjInCond) {
        return false;
    }

    std::vector<int> index(numberObjects);

    for (int i = 0; i < numberObjects; ++i) {
        index[i] = i;
    }

    int totalLoops = 0;
    int passLoops = 0;

    // condition result condResult set to true if at least one permutation
    //     passes all requirements
    // all possible permutations are checked
    bool condResult = false;

    // store the indices of the calorimeter objects
    // from the combination evaluated in the condition
    SingleCombInCond objectsInComb;
    objectsInComb.reserve(nObjInCond);

    // clear the m_combinationsInCond vector
    combinationsInCond().clear();



    ////// NEW Method
    if( nObjInCond==1 ){

      // clear the indices in the combination
      objectsInComb.clear();

      for( int i=0; i<numberObjects; i++ ){

	totalLoops++;
	bool passCondition = checkObjectParameter(0, *(candVec->at(useBx,i)));
	if( passCondition ){
	  objectsInComb.push_back(i);
	  condResult = true;
	  passLoops++;
	  combinationsInCond().push_back(objectsInComb);
	}
      }
    }
    else if( nObjInCond==2 ){

      // clear the indices in the combination
      objectsInComb.clear();

      for( int i=0; i<numberObjects; i++ ){
	bool passCondition0i = checkObjectParameter(0, *(candVec->at(useBx,i)));
	bool passCondition1i = checkObjectParameter(1, *(candVec->at(useBx,i)));

	if( !( passCondition0i || passCondition1i ) ) continue;

	for( int j=0; j<numberObjects; j++ ){
	  if( i==j ) continue;
	  totalLoops++;

	  bool passCondition0j = checkObjectParameter(0, *(candVec->at(useBx,j)));
	  bool passCondition1j = checkObjectParameter(1, *(candVec->at(useBx,j)));

	  bool pass = ( 
		       (passCondition0i && passCondition1j) ||
		       (passCondition0j && passCondition1i)
		       );

	  if( pass ){

	    if (m_gtCaloTemplate->wsc()) {

	      // wsc requirements have always nObjInCond = 2
	      // one can use directly 0] and 1] to compute
	      // eta and phi differences
	      const int ObjInWscComb = 2;
	      if (nObjInCond != ObjInWscComb) {

                if (m_verbosity) {
		  edm::LogError("L1GlobalTrigger")
		    << "\n  Error: "
		    << "number of particles in condition with spatial correlation = "
		    << nObjInCond << "\n  it must be = " << ObjInWscComb
		    << std::endl;
                }

                continue;
	      }

	      GlobalCaloTemplate::CorrelationParameter corrPar =
                *(m_gtCaloTemplate->correlationParameter());

	      // check delta eta
	      if( !checkRangeDeltaEta( (candVec->at(useBx,i))->hwEta(), (candVec->at(useBx,j))->hwEta(), corrPar.deltaEtaRangeLower, corrPar.deltaEtaRangeUpper) ){
		LogDebug("l1t|Global") << "\t\t l1t::Candidate failed checkRangeDeltaEta" << std::endl;
		continue;
	      }

	      // check delta phi
	      if( !checkRangeDeltaPhi( (candVec->at(useBx,i))->hwPhi(), (candVec->at(useBx,j))->hwPhi(), 
				       corrPar.deltaPhiRangeLower, corrPar.deltaPhiRangeUpper) ){
		LogDebug("l1t|Global") << "\t\t l1t::Candidate failed checkRangeDeltaPhi" << std::endl;
		continue;
	      }

	    } // end wsc check



	    objectsInComb.push_back(i);
	    objectsInComb.push_back(j);
	    condResult = true;
	    passLoops++;
	    combinationsInCond().push_back(objectsInComb);
	  }
	}
      }
    }
    else if( nObjInCond==3 ){

      // clear the indices in the combination
      objectsInComb.clear();

      for( int i=0; i<numberObjects; i++ ){
	bool passCondition0i = checkObjectParameter(0, *(candVec->at(useBx,i)));
	bool passCondition1i = checkObjectParameter(1, *(candVec->at(useBx,i)));
	bool passCondition2i = checkObjectParameter(2, *(candVec->at(useBx,i)));

	if( !( passCondition0i || passCondition1i || passCondition2i ) ) continue;

	for( int j=0; j<numberObjects; j++ ){
	  if( i==j ) continue;

	  bool passCondition0j = checkObjectParameter(0, *(candVec->at(useBx,j)));
	  bool passCondition1j = checkObjectParameter(1, *(candVec->at(useBx,j)));
	  bool passCondition2j = checkObjectParameter(2, *(candVec->at(useBx,j)));

	  if( !( passCondition0j || passCondition1j || passCondition2j ) ) continue;

	  for( int k=0; k<numberObjects; k++ ){
	    if( k==i || k==j ) continue;
	    totalLoops++;

	    bool passCondition0k = checkObjectParameter(0, *(candVec->at(useBx,k)));
	    bool passCondition1k = checkObjectParameter(1, *(candVec->at(useBx,k)));
	    bool passCondition2k = checkObjectParameter(2, *(candVec->at(useBx,k)));

	    bool pass = ( 
			 (passCondition0i && passCondition1j && passCondition2k) ||
			 (passCondition0i && passCondition1k && passCondition2j) ||
			 (passCondition0j && passCondition1k && passCondition2i) ||
			 (passCondition0j && passCondition1i && passCondition2k) ||
			 (passCondition0k && passCondition1i && passCondition2j) ||
			 (passCondition0k && passCondition1j && passCondition2i)
			 );
	    if( pass ){
	      condResult = true;
	      passLoops++;
	      objectsInComb.push_back(i);
	      objectsInComb.push_back(j);
	      objectsInComb.push_back(k);
 	      combinationsInCond().push_back(objectsInComb);
	    }
	  }// end loop on k
	}// end loop on j
      }// end loop on i
    } // end if condition has 3 objects
    else if( nObjInCond==4 ){

      // clear the indices in the combination
      objectsInComb.clear();

      for( int i=0; i<numberObjects; i++ ){
	bool passCondition0i = checkObjectParameter(0, *(candVec->at(useBx,i)));
	bool passCondition1i = checkObjectParameter(1, *(candVec->at(useBx,i)));
	bool passCondition2i = checkObjectParameter(2, *(candVec->at(useBx,i)));
	bool passCondition3i = checkObjectParameter(3, *(candVec->at(useBx,i)));

	if( !( passCondition0i || passCondition1i || passCondition2i || passCondition3i ) ) continue;

	for( int j=0; j<numberObjects; j++ ){
	  if( j==i ) continue;

	  bool passCondition0j = checkObjectParameter(0, *(candVec->at(useBx,j)));
	  bool passCondition1j = checkObjectParameter(1, *(candVec->at(useBx,j)));
	  bool passCondition2j = checkObjectParameter(2, *(candVec->at(useBx,j)));
	  bool passCondition3j = checkObjectParameter(3, *(candVec->at(useBx,j)));

	  if( !( passCondition0j || passCondition1j || passCondition2j || passCondition3j ) ) continue;

	  for( int k=0; k<numberObjects; k++ ){
	    if( k==i || k==j ) continue;

	    bool passCondition0k = checkObjectParameter(0, *(candVec->at(useBx,k)));
	    bool passCondition1k = checkObjectParameter(1, *(candVec->at(useBx,k)));
	    bool passCondition2k = checkObjectParameter(2, *(candVec->at(useBx,k)));
	    bool passCondition3k = checkObjectParameter(3, *(candVec->at(useBx,k)));

	    if( !( passCondition0k || passCondition1k || passCondition2k || passCondition3k ) ) continue;
	    
	    for( int m=0; m<numberObjects; m++ ){
	      if( m==i || m==j || m==k ) continue;
	      totalLoops++;

	      bool passCondition0m = checkObjectParameter(0, *(candVec->at(useBx,m)));
	      bool passCondition1m = checkObjectParameter(1, *(candVec->at(useBx,m)));
	      bool passCondition2m = checkObjectParameter(2, *(candVec->at(useBx,m)));
	      bool passCondition3m = checkObjectParameter(3, *(candVec->at(useBx,m)));

	      bool pass = ( 
			   (passCondition0i && passCondition1j && passCondition2k && passCondition3m) ||
			   (passCondition0i && passCondition1j && passCondition2m && passCondition3k) ||
			   (passCondition0i && passCondition1k && passCondition2j && passCondition3m) ||
			   (passCondition0i && passCondition1k && passCondition2m && passCondition3j) ||
			   (passCondition0i && passCondition1m && passCondition2j && passCondition3k) ||
			   (passCondition0i && passCondition1m && passCondition2k && passCondition3j) ||
			   (passCondition0j && passCondition1i && passCondition2k && passCondition3m) ||
			   (passCondition0j && passCondition1i && passCondition2m && passCondition3k) ||
			   (passCondition0j && passCondition1k && passCondition2i && passCondition3m) ||
			   (passCondition0j && passCondition1k && passCondition2m && passCondition3i) ||
			   (passCondition0j && passCondition1m && passCondition2i && passCondition3k) ||
			   (passCondition0j && passCondition1m && passCondition2k && passCondition3i) ||
			   (passCondition0k && passCondition1i && passCondition2j && passCondition3m) ||
			   (passCondition0k && passCondition1i && passCondition2m && passCondition3j) ||
			   (passCondition0k && passCondition1j && passCondition2i && passCondition3m) ||
			   (passCondition0k && passCondition1j && passCondition2m && passCondition3i) ||
			   (passCondition0k && passCondition1m && passCondition2i && passCondition3j) ||
			   (passCondition0k && passCondition1m && passCondition2j && passCondition3i) ||
			   (passCondition0m && passCondition1i && passCondition2j && passCondition3k) ||
			   (passCondition0m && passCondition1i && passCondition2k && passCondition3j) ||
			   (passCondition0m && passCondition1j && passCondition2i && passCondition3k) ||
			   (passCondition0m && passCondition1j && passCondition2k && passCondition3i) ||
			   (passCondition0m && passCondition1k && passCondition2i && passCondition3j) ||
			   (passCondition0m && passCondition1k && passCondition2j && passCondition3i)
			   );
	      if( pass ){
		objectsInComb.push_back(i);
		objectsInComb.push_back(j);
		objectsInComb.push_back(k);
		objectsInComb.push_back(m);
		condResult = true;
		passLoops++;
		combinationsInCond().push_back(objectsInComb);
	      }
	    }// end loop on m
	  }// end loop on k
	}// end loop on j
      }// end loop on i
    } // end if condition has 4 objects


    LogTrace("L1GlobalTrigger")
       << "\n  GtCaloCondition: total number of permutations found:          " << totalLoops
       << "\n  GtCaloCondition: number of permutations passing requirements: " << passLoops
       << "\n" << std::endl;

    return condResult;

}

// load calo candidates
const l1t::L1Candidate* l1t::GtCaloCondition::getCandidate(const int bx, const int indexCand) const {

    // objectType() gives the type for nrObjects() only,
    // but in a CondCalo all objects have the same type
    // take type from the type of the first object
    switch ((m_gtCaloTemplate->objectType())[0]) {
        case NoIsoEG:
            return (m_uGtB->getCandL1EG())->at(bx,indexCand);
            break;

        case CenJet:
            return (m_uGtB->getCandL1Jet())->at(bx,indexCand);
            break;

       case TauJet:
            return (m_uGtB->getCandL1Tau())->at(bx,indexCand);
            break;
        default:
            return 0;
            break;
    }

    return 0;
}

/**
 * checkObjectParameter - Compare a single particle with a numbered condition.
 *
 * @param iCondition The number of the condition.
 * @param cand The candidate to compare.
 *
 * @return The result of the comparison (false if a condition does not exist).
 */

const bool l1t::GtCaloCondition::checkObjectParameter(const int iCondition, const l1t::L1Candidate& cand) const {

    // number of objects in condition
    int nObjInCond = m_gtCaloTemplate->nrObjects();

    if (iCondition >= nObjInCond || iCondition < 0) {
        return false;
    }

    // empty candidates can not be compared
//     if (cand.empty()) {
//         return false;
//     }

    const GlobalCaloTemplate::ObjectParameter objPar = ( *(m_gtCaloTemplate->objectParameter()) )[iCondition];

    LogDebug("l1t|Global")
      << "\n GlobalCaloTemplate: "
      << "\n\t condRelativeBx = " << m_gtCaloTemplate->condRelativeBx()
      << "\n ObjectParameter : "
      << "\n\t etThreshold = " << objPar.etThreshold
      << "\n\t etaRange    = " << objPar.etaRange
      << "\n\t phiRange    = " << objPar.phiRange
      << std::endl;

    LogDebug("l1t|Global")
      << "\n l1t::Candidate : "
      << "\n\t hwPt   = " <<  cand.hwPt()
      << "\n\t hwEta  = " << cand.hwEta()
      << "\n\t hwPhi  = " << cand.hwPhi()
      << std::endl;


    // check energy threshold
    if ( !checkThreshold(objPar.etThreshold, cand.hwPt(), m_gtCaloTemplate->condGEq()) ) {
      LogDebug("l1t|Global") << "\t\t l1t::Candidate failed checkThreshold" << std::endl;
        return false;
    }

    // check eta
    if( !checkRangeEta(cand.hwEta(), objPar.etaWindowLower, objPar.etaWindowUpper, objPar.etaWindowVetoLower, objPar.etaWindowVetoLower) ){
      LogDebug("l1t|Global") << "\t\t l1t::Candidate failed checkRange(eta)" << std::endl;
      return false;
    }

//     if (!checkBit(objPar.etaRange, cand.hwEta())) {
//         return false;
//     }

    // check phi
    if( !checkRangePhi(cand.hwPhi(), objPar.phiWindowLower, objPar.phiWindowUpper, objPar.phiWindowVetoLower, objPar.phiWindowVetoLower) ){
      LogDebug("l1t|Global") << "\t\t l1t::Candidate failed checkRange(phi)" << std::endl;
      return false;
    }

//     if (!checkBit(objPar.phiRange, cand.hwPhi())) {
//         return false;
//     }

    // particle matches if we get here
    //LogTrace("L1GlobalTrigger")
    //    << "  checkObjectParameter: calorimeter object OK, passes all requirements\n"
    //    << std::endl;

    return true;
}

void l1t::GtCaloCondition::print(std::ostream& myCout) const {

    m_gtCaloTemplate->print(myCout);

    myCout << "    Number of bits for eta of calorimeter objects = "
            << m_ifCaloEtaNumberBits << std::endl;
    myCout << "    Maximum number of bins for the delta phi scales = "
            << m_corrParDeltaPhiNrBins << "\n " << std::endl;

    ConditionEvaluation::print(myCout);

}

