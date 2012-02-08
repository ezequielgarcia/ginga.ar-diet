/******************************************************************************
Este arquivo eh parte da implementacao do ambiente declarativo do middleware
Ginga (Ginga-NCL).

Direitos Autorais Reservados (c) 1989-2007 PUC-Rio/Laboratorio TeleMidia

Este programa eh software livre; voce pode redistribui-lo e/ou modificah-lo sob
os termos da Licenca Publica Geral GNU versao 2 conforme publicada pela Free
Software Foundation.

Este programa eh distribuido na expectativa de que seja util, porem, SEM
NENHUMA GARANTIA; nem mesmo a garantia implicita de COMERCIABILIDADE OU
ADEQUACAO A UMA FINALIDADE ESPECIFICA. Consulte a Licenca Publica Geral do
GNU versao 2 para mais detalhes.

Voce deve ter recebido uma copia da Licenca Publica Geral do GNU versao 2 junto
com este programa; se nao, escreva para a Free Software Foundation, Inc., no
endereco 59 Temple Street, Suite 330, Boston, MA 02111-1307 USA.

Para maiores informacoes:
ncl @ telemidia.puc-rio.br
http://www.ncl.org.br
http://www.ginga.org.br
http://www.telemidia.puc-rio.br
******************************************************************************
This file is part of the declarative environment of middleware Ginga (Ginga-NCL)

Copyright: 1989-2007 PUC-RIO/LABORATORIO TELEMIDIA, All Rights Reserved.
  Copyright 2010 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata


This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License version 2 as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License version 2 for more
details.

You should have received a copy of the GNU General Public License version 2
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

For further information contact:
ncl @ telemidia.puc-rio.br
http://www.ncl.org.br
http://www.ginga.org.br
http://www.telemidia.puc-rio.br
*******************************************************************************/

#include "../../../include/ncl/NclConnectorsConverter.h"
#include "../../../include/ncl/NclDocumentConverter.h"
#include "system/util/functions.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace converter {
namespace ncl {
	NclConnectorsConverter::NclConnectorsConverter(
		    DocumentParser* documentParser) : NclConnectorsParser(
		    	    documentParser) {


	}

	void NclConnectorsConverter::addCausalConnectorToConnectorBase(
		    void *parentObject, void *childObject) {

		((ConnectorBase*)parentObject)->addConnector((Connector*)childObject);
	}

	void NclConnectorsConverter::addConnectorParamToCausalConnector(
		    void *parentObject, void *childObject) {

		((Connector*)parentObject)->addParameter((Parameter*)childObject);
	}

	void NclConnectorsConverter::addImportBaseToConnectorBase(
		    void *parentObject, void *childObject) {

		string baseAlias, baseLocation;
		NclDocumentConverter *compiler;
		NclDocument *importedDocument;
		ConnectorBase *connectorBase;

		// get the external base alias and location
		baseAlias = XMLString::transcode(((DOMElement*)childObject)->
			    getAttribute(XMLString::transcode("alias")));

		baseLocation = XMLString::transcode(((DOMElement*)childObject)->
			    getAttribute(XMLString::transcode("documentURI")));

		compiler = (NclDocumentConverter*)getDocumentParser();
		importedDocument = compiler->importDocument(baseLocation);
		if (importedDocument == NULL) {
			return;
		}

		connectorBase = importedDocument->getConnectorBase();
		if (connectorBase == NULL) {
			return;
		}

		// insert the imported base into the document connector base
		((ConnectorBase*)parentObject)->addBase(
			    connectorBase, baseAlias, baseLocation);

		/*
		 * Hashtable connBaseimports = new Hashtable();
		 * connBaseimports->put(baseAlias,baseLocation);
		 * getDocumentParser()->addObject("return","ConnectorImports",
		 * connBaseimports);
		 */
	}

	void *NclConnectorsConverter::createCausalConnector(
		    DOMElement *parentElement, void *objGrandParent) {

		string connectorId = "";
		/*
		 * if (connectorUri->equalsIgnoreCase("")) { //se nao tiver uma uri do
		 * arquivo do conector, atribuir somente o id
		 * do elemento conector como id do conector
		 * connectorId = parentElement->getAttribute("id"); connectorId =
		 * "#" + parentElement->getAttribute("id"); } else { //atribuir a id do
		 * conector como sendo a uri do seu arquivo
		 * connectorId = connectorUri;
		 */
		connectorId = XMLString::transcode(parentElement->getAttribute(
			    XMLString::transcode("id")));

		connector = new CausalConnector(connectorId);
		return connector;
	}

	void *NclConnectorsConverter::createConnectorBase(
		    DOMElement *parentElement, void *objGrandParent) {

		ConnectorBase *connBase;
		// criar nova base de conectores com id gerado a partir do nome de seu
		// elemento
		connBase = new ConnectorBase(XMLString::transcode(
			    parentElement->getAttribute(XMLString::transcode("id"))));

		return connBase;
	}

	void *NclConnectorsConverter::createConnectorParam(
		    DOMElement *parentElement, void *objGrandParent) {

		Parameter *parameter;
		parameter = new Parameter(
			    XMLString::transcode(parentElement->getAttribute(
			     	     XMLString::transcode("name"))),

			    XMLString::transcode(parentElement->getAttribute(
			    	     XMLString::transcode("type"))));

		return parameter;
	}

	void NclConnectorsConverter::compileRoleInformation(
		    Role *role, DOMElement *parentElement) {

		string attValue;
		// event type
		if (parentElement->hasAttribute(XMLString::transcode("eventType"))) {
			attValue = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("eventType")));

			role->setEventType(EventUtil::getTypeCode(attValue));
		}

		//  cardinality
		if (parentElement->hasAttribute(XMLString::transcode("min"))) {
			attValue = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("min")));

			((Role*)role)->setMinCon((atoi(attValue.c_str())));
		}

		if (parentElement->hasAttribute(XMLString::transcode("max"))) {
			attValue = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("max")));

			if (XMLString::compareIString(attValue.c_str(), "unbounded")==0) {
				((Role*)role)->setMaxCon(Role::UNBOUNDED);

			} else {
				((Role*)role)->setMaxCon(atoi(attValue.c_str()));
			}
		}
	}

	void *NclConnectorsConverter::createSimpleCondition(
		    DOMElement *parentElement, void *objGrandParent) {

		SimpleCondition* conditionExpression;
		string attValue;

		string roleLabel;
		roleLabel = XMLString::transcode(parentElement->getAttribute(
			    XMLString::transcode("role")));

		conditionExpression = new SimpleCondition(roleLabel);

		compileRoleInformation(conditionExpression, parentElement);

		// transition
		if (parentElement->hasAttribute(XMLString::transcode("transition"))) {
			attValue = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("transition")));

			conditionExpression->setTransition(
				    EventUtil::getTransitionCode(attValue));
		}

		// parametro
		if (conditionExpression->getEventType() == EventUtil::EVT_SELECTION) {
			if (parentElement->hasAttribute(XMLString::transcode("key"))) {
				attValue = XMLString::transcode(parentElement->getAttribute(
					    XMLString::transcode("key")));

				conditionExpression->setKey(attValue);
			}
		}

		// qualifier
		if (parentElement->hasAttribute(XMLString::transcode(
			    "qualifier"))) {

			attValue = XMLString::transcode(parentElement->
				    getAttribute(XMLString::transcode("qualifier")));

			if (attValue == "or") {
				conditionExpression->setQualifier(
					    CompoundCondition::OP_OR);

			} else {
				conditionExpression->setQualifier(
					    CompoundCondition::OP_AND);
			}
		}

		//testar delay
		if (parentElement->hasAttribute(XMLString::transcode("delay"))) {
			attValue = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("delay")));

			if (attValue[0] == '$') {
				conditionExpression->setDelay( attValue );

			} else {
				double delayValue;
				delayValue = stof(attValue.substr(0, (attValue.length() - 1)))
					    * 1000;

				conditionExpression->setDelay(itos(delayValue));
			}
		}

		// retornar expressao de condicao
		return conditionExpression;
	}

	void *NclConnectorsConverter::createCompoundCondition(
		    DOMElement *parentElement, void *objGrandParent) {

		CompoundCondition *conditionExpression;
		string attValue;

		conditionExpression = new CompoundCondition();

		if (XMLString::compareIString(
			    XMLString::transcode(parentElement->getAttribute(
			    	    XMLString::transcode("operator"))),

			    "and") == 0) {

			conditionExpression->setOperator(CompoundCondition::OP_AND);

		} else {
			conditionExpression->setOperator(CompoundCondition::OP_OR);
		}

		//  testar delay
		if (parentElement->hasAttribute(XMLString::transcode("delay"))) {
			attValue = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("delay")));

			if (attValue[0] == '$') {
				conditionExpression->setDelay( attValue );

			} else {
				double delayValue = stof(attValue.substr(
					    0, (attValue.length() - 1))) * 1000;

				conditionExpression->setDelay( itos(delayValue) );
			}
		}

		// retornar expressao de condicao
		return conditionExpression;
	}

	void* NclConnectorsConverter::createAttributeAssessment(
		    DOMElement *parentElement, void *objGrandParent) {

		AttributeAssessment *attributeAssessment;
		string attValue;

		string roleLabel;
		roleLabel = XMLString::transcode(parentElement->getAttribute(
			   XMLString::transcode("role")));

		attributeAssessment = new AttributeAssessment(roleLabel);

		// event type
		if (parentElement->hasAttribute(XMLString::transcode("eventType"))) {
			attValue = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("eventType")));

			attributeAssessment->setEventType(
				    EventUtil::getTypeCode(attValue));
		}

		// event type
		if (parentElement->hasAttribute(XMLString::transcode(
			     "attributeType"))) {

			attValue = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("attributeType")));

			attributeAssessment->setAttributeType(
				    EventUtil::getAttributeTypeCode(attValue));
		}

		// parameter
		if (attributeAssessment->getEventType() == EventUtil::EVT_SELECTION) {
			if (parentElement->hasAttribute(XMLString::transcode("key"))) {
				attValue = XMLString::transcode(parentElement->getAttribute(
					    XMLString::transcode("key")));

				attributeAssessment->setKey(attValue);
			}
		}

		//testing offset
		if (parentElement->hasAttribute(XMLString::transcode("offset"))) {
			attValue = XMLString::transcode(parentElement->getAttribute(
				     XMLString::transcode("offset")));

			attributeAssessment->setOffset(attValue);
		}

		return attributeAssessment;
	}

	void *NclConnectorsConverter::createValueAssessment(
		    DOMElement *parentElement, void *objGrandParent) {

		string attValue;

		attValue = XMLString::transcode(parentElement->getAttribute(
			    XMLString::transcode("value")));

		return new ValueAssessment(attValue);
	}

	void *NclConnectorsConverter::createAssessmentStatement(
		     DOMElement *parentElement, void *objGrandParent) {

		AssessmentStatement *assessmentStatement;
		string attValue;

		if (parentElement->hasAttribute(XMLString::transcode("comparator"))) {
			attValue = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("comparator")));

			assessmentStatement = new AssessmentStatement(
				    brutil::Comparator::fromString(attValue));

		} else {
			assessmentStatement = new AssessmentStatement(brutil::Comparator::CMP_EQ);
		}

		return assessmentStatement;
	}

	void *NclConnectorsConverter::createCompoundStatement(
		    DOMElement *parentElement, void *objGrandParent) {

		CompoundStatement *compoundStatement;
		string attValue;

		compoundStatement = new CompoundStatement();

		if (XMLString::compareIString(XMLString::transcode(
			    parentElement->getAttribute(
			    	    XMLString::transcode("operator"))),"and") == 0) {

			compoundStatement->setOperator(CompoundStatement::OP_AND);

		} else {
			compoundStatement->setOperator(CompoundStatement::OP_OR);
		}

		// testing isNegated
		if (parentElement->hasAttribute(XMLString::transcode("isNegated"))) {
			attValue = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("isNegated")));

			compoundStatement->setNegated(XMLString::compareIString(
				     attValue.c_str(), "true") == 0);
		}

		return compoundStatement;
	}

	void *NclConnectorsConverter::createSimpleAction(
		     DOMElement *parentElement, void *objGrandParent) {

	    SimpleAction *actionExpression;
	    string attValue;

	    attValue = XMLString::transcode(
	    		parentElement->getAttribute(XMLString::transcode("role")));

		actionExpression = new SimpleAction(attValue);

		//animation
		if (attValue == "set") {
			Animation* animation = NULL;
			if (parentElement->hasAttribute(
						XMLString::transcode("duration"))) {

				attValue = XMLString::transcode(parentElement->
						getAttribute(XMLString::transcode("duration")));

				animation = new Animation();
				if (attValue[0] != '$') {
				    if (attValue != "" && attValue.find_last_of('s') == attValue.length() - 1 )
                        attValue = attValue.substr(0, attValue.length() - 1);
                    if (! ::util::isNumeric((void*)attValue.c_str())){
                        cout << "[NclConnectorsConverter] Warning! Invalid parameter (duration) for animation" << endl;
                        attValue = "0";
                    }
				}
				animation->setDuration(attValue);
			}

			if (parentElement->hasAttribute(
						XMLString::transcode("by"))) {

				attValue = XMLString::transcode(parentElement->
						getAttribute(XMLString::transcode("by")));


				if (animation == NULL) {
					animation = new Animation();
				}
				if (attValue[0] == '$' || isNumeric((void*)attValue.c_str()))
                    animation->setBy(attValue);
                else{
                    cout << "[NclConnectorsConverter] Warning! Invalid parameter (by) for animation" << endl;
                    animation->setBy("0");
                }
            }
			actionExpression->setAnimation(animation);
		}

		compileRoleInformation(actionExpression, parentElement);

		// transition
		if (parentElement->hasAttribute(XMLString::transcode("actionType"))) {
			attValue = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("actionType")));

			actionExpression->setActionType(convertActionType(attValue));
		}

		if (parentElement->hasAttribute(XMLString::transcode("qualifier"))) {
			if (XMLString::compareIString(XMLString::transcode(
				     parentElement->getAttribute(
				     	    XMLString::transcode("qualifier"))),"seq") == 0) {

				actionExpression->setQualifier(CompoundAction::OP_SEQ);

			} else { // any
				actionExpression->setQualifier(CompoundAction::OP_PAR);
			}
		}

		//testing delay
		if (parentElement->hasAttribute(XMLString::transcode("delay"))) {
			attValue = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("delay")));

			if (attValue[0] == '$') {
				actionExpression->setDelay(attValue);

			} else {
				actionExpression->setDelay(itos(stof(
					    attValue.substr(0, attValue.length() - 1)) * 1000));
			}
		}

		//  testing repeatDelay
		if (parentElement->hasAttribute(XMLString::transcode("repeatDelay"))) {
			attValue = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("repeatDelay")));

			actionExpression->setDelay(attValue);
			if (attValue[0] == '$') {
				actionExpression->setDelay(attValue);

			} else {
				actionExpression->setDelay(itos(stof(attValue.substr(
					    0, attValue.length() - 1)) * 1000));
			}
		}

		// repeat
		if (parentElement->hasAttribute(XMLString::transcode("repeat"))) {
			attValue = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("repeat")));

			if (XMLString::compareIString(attValue.c_str(), "infinite") == 0) {
				actionExpression->setRepeat(itos(2^30));

			} else {
				actionExpression->setRepeat(attValue);
			}
		}

		// testing value
		if (parentElement->hasAttribute(XMLString::transcode("value"))) {
			attValue = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("value")));

			actionExpression->setValue(attValue);
		}

		// returning action expression
		return actionExpression;
	}

	void *NclConnectorsConverter::createCompoundAction(
		    DOMElement *parentElement, void *objGrandParent) {

		CompoundAction *actionExpression;
		string attValue;

		actionExpression = new CompoundAction();

		if (XMLString::compareIString(XMLString::transcode(
			    parentElement->getAttribute(XMLString::transcode("operator"))),
			    	    "seq") == 0) {

			actionExpression->setOperator(CompoundAction::OP_SEQ);

		} else {
			actionExpression->setOperator(CompoundAction::OP_PAR);
		}

		//  testar delay
		if (parentElement->hasAttribute(XMLString::transcode("delay"))) {
			attValue = XMLString::transcode(parentElement->getAttribute(
				     XMLString::transcode("delay")));

			if (attValue[0]=='$') {
				actionExpression->setDelay(attValue);

			} else {
				actionExpression->setDelay(itos(stof(attValue.substr(
					    0, attValue.length() - 1)) * 1000));
			}
		}

		// retornar expressao de condicao
		return actionExpression;
	}

	Parameter *NclConnectorsConverter::getParameter(string paramName) {
		return (Parameter*) (connector->getParameter(paramName));
	}

	short NclConnectorsConverter::convertActionType(string actionType) {
		if (actionType=="start")
			return SimpleAction::ACT_START;

		else if (actionType=="stop")
			return SimpleAction::ACT_STOP;

		else if (actionType=="set")
			return SimpleAction::ACT_SET;

		else if (actionType=="pause")
			return SimpleAction::ACT_PAUSE;

		else if (actionType=="resume")
			return SimpleAction::ACT_RESUME;

		else if (actionType=="abort")
			return SimpleAction::ACT_ABORT;

		return -1;
	}

	short NclConnectorsConverter::convertEventState(string eventState) {
		if (eventState=="occurring") {
			return EventUtil::ST_OCCURRING;

		} else if (eventState=="paused") {
			return EventUtil::ST_PAUSED;

		} else if (eventState=="sleeping") {
			return EventUtil::ST_SLEEPING;

		}

		return -1;
	}

	void NclConnectorsConverter::addSimpleConditionToCompoundCondition(
		    void *parentObject, void *childObject) {

		((CompoundCondition*)parentObject)->addConditionExpression(
			    (ConditionExpression*)childObject);
	}

	void NclConnectorsConverter::addCompoundConditionToCompoundCondition(
		    void *parentObject, void *childObject) {

		((CompoundCondition*)parentObject)->addConditionExpression(
			    (ConditionExpression*)childObject);
	}

	void NclConnectorsConverter::addAssessmentStatementToCompoundCondition(
		    void *parentObject, void *childObject) {

		((CompoundCondition*)parentObject)->addConditionExpression(
			    (ConditionExpression*)childObject);
	}

	void NclConnectorsConverter::addCompoundStatementToCompoundCondition(
		    void *parentObject, void *childObject) {

		((CompoundCondition*)parentObject)->addConditionExpression(
			    (ConditionExpression*)childObject);
	}

	void NclConnectorsConverter::addAttributeAssessmentToAssessmentStatement(
		    void *parentObject, void *childObject) {

		AssessmentStatement *statement;

		statement = (AssessmentStatement*)parentObject;
		if (statement->getMainAssessment() == NULL) {
			statement->setMainAssessment((AttributeAssessment*)childObject);

		} else {
			statement->setOtherAssessment((AttributeAssessment*)childObject);
		}
	}

	void NclConnectorsConverter::addValueAssessmentToAssessmentStatement(
		    void *parentObject, void *childObject) {

		((AssessmentStatement*)parentObject)->setOtherAssessment(
			   (ValueAssessment*)childObject);
	}

	void NclConnectorsConverter::addAssessmentStatementToCompoundStatement(
		    void *parentObject, void *childObject) {

		((CompoundStatement*)parentObject)->addStatement(
			   (Statement*)childObject);
	}

	void NclConnectorsConverter::addCompoundStatementToCompoundStatement(
		    void *parentObject, void *childObject) {

		((CompoundStatement*)parentObject)->addStatement(
			    (Statement*)childObject);
	}

	void NclConnectorsConverter::addSimpleActionToCompoundAction(
		    void *parentObject, void *childObject) {

		((CompoundAction*)parentObject)->addAction((Action*)childObject);
	}

	void NclConnectorsConverter::addCompoundActionToCompoundAction(
		    void *parentObject, void *childObject) {

		((CompoundAction*)parentObject)->addAction((Action*)childObject);
	}

	void NclConnectorsConverter::addSimpleConditionToCausalConnector(
		    void *parentObject, void *childObject) {

		((CausalConnector*)parentObject)->setConditionExpression(
			    (ConditionExpression*)childObject);
	}

	void NclConnectorsConverter::addCompoundConditionToCausalConnector(
		    void *parentObject, void *childObject) {

		((CausalConnector*)parentObject)->setConditionExpression(
			    (ConditionExpression*)childObject);
	}

	void NclConnectorsConverter::addSimpleActionToCausalConnector(
		    void *parentObject, void *childObject) {

		((CausalConnector*)parentObject)->setAction((Action*)childObject);
	}

	void NclConnectorsConverter::addCompoundActionToCausalConnector(
		    void *parentObject, void *childObject) {

		((CausalConnector*)parentObject)->setAction((Action*)childObject);
	}

	void NclConnectorsConverter::addAssessmentStatementToConstraintConnector(
		    void *parentObject, void *childObject) {

		// TODO Auto-generated method stub
	}

	void NclConnectorsConverter::addCompoundStatementToConstraintConnector(
		    void *parentObject, void *childObject) {

		// TODO Auto-generated method stub
	}

	void NclConnectorsConverter::addConstraintConnectorToConnectorBase(
		    void *parentObject, void *childObject) {

		// TODO Auto-generated method stub
	}

	void *NclConnectorsConverter::createConstraintConnector(
		    DOMElement *parentElement, void *objGrandParent) {

		// TODO Auto-generated method stub
		return NULL;
	}
}
}
}
}
}
