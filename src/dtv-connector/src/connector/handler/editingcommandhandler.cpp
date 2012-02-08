/*******************************************************************************

  Copyright (C) 2010, 2011 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata

********************************************************************************

  This file is part of DTV-connector implementation.

    DTV-connector is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation, either version 2 of the License.

    DTV-connector is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
  this program. If not, see <http://www.gnu.org/licenses/>.

********************************************************************************

  Este archivo es parte de la implementación de DTV-connector.

    DTV-connector es Software Libre: Ud. puede redistribuirlo y/o modificarlo
  bajo los términos de la Licencia Pública General Reducida GNU como es publicada por la
  Free Software Foundation, según la versión 2 de la licencia.

    DTV-connector se distribuye esperando que resulte de utilidad, pero SIN NINGUNA
  GARANTÍA; ni siquiera la garantía implícita de COMERCIALIZACIÓN o ADECUACIÓN
  PARA ALGÚN PROPÓSITO PARTICULAR. Para más detalles, revise la Licencia Pública
  General Reducida GNU.

    Ud. debería haber recibido una copia de la Licencia Pública General Reducida GNU
  junto a este programa. Si no, puede verla en <http://www.gnu.org/licenses/>.

*******************************************************************************/
#include "editingcommandhandler.h"
#include "../connector.h"
#include "../../demuxer/psi/dsmcc/parser.h"
#include <util/buffer.h>
#include <util/types.h>
#include <util/functions.h>

#include <iostream>

namespace connector {

EditingCommandHandler::EditingCommandHandler( void )
{
}

EditingCommandHandler::~EditingCommandHandler( void )
{
        std::map< ::util::BYTE , EditingCommandData*>::iterator it;

        it = _incompleteEditingCommands.begin();
        for ( ; it != _incompleteEditingCommands.end(); ++it)
                delete (*it).second;
        _incompleteEditingCommands.clear();
}

void EditingCommandHandler::send( Connector *conn, EditingCommandData* data )
{
        char buf[257 + MESSAGE_HEADER]; // 255 StreamEvent max payload size + descriptor tag + descriptor length

        const int DESCRIPTOR_LENGTH_IDX = MESSAGE_HEADER +  1;
        const int PRIVDATA_LENGTH_IDX   = MESSAGE_HEADER + 12;
        const int SEQNUM_IDX            = MESSAGE_HEADER + 14;
        const int PAYLOAD_IDX           = MESSAGE_HEADER + 15;

        // Make payload
        // 8 bits descriptor tag
        buf[MESSAGE_HEADER] = DSMCC_SE_GINGA_EDITCMD; // STREAM EVENT DESCRIPTOR

        // 16 bits eventID
        unsigned int eid = data->getEventID();
        buf[MESSAGE_HEADER+2] = eid >> 8;
        buf[MESSAGE_HEADER+3] = eid & 0xFF;

        // 31 bits reserved + 33 bits NTP
        buf[MESSAGE_HEADER+4] = 0xFF;
        buf[MESSAGE_HEADER+5] = 0xFF;
        buf[MESSAGE_HEADER+6] = 0xFF;

        unsigned long long npt = data->getNPT();
        buf[MESSAGE_HEADER+ 7] =(npt >> 32) | 0xFE;
        buf[MESSAGE_HEADER+ 8] =(npt >> 24) & 0xFF;
        buf[MESSAGE_HEADER+ 9] =(npt >> 16) & 0xFF;
        buf[MESSAGE_HEADER+10] =(npt >>  8) & 0xFF;
        buf[MESSAGE_HEADER+11] =(npt      ) & 0xFF;

        // 8 bits commandTag
        buf[MESSAGE_HEADER+13] = data->getCommandTag();

        int plsize = data->getPayloadSize();
        const unsigned char* payload = data->getPayload();
        ::util::BYTE seqNumber = 0;

        if (plsize > MAX_PAYLOAD_SIZE) {
                std::cerr << "EditingCommandHandler::send : Can't send such a long message!" << std::endl;
                return;
        }

        char* payload_ptr = buf+PAYLOAD_IDX;
        while (plsize > 0) {
                ::util::BYTE plsize_1 = std::min(plsize, 241); // Max allowed per packet...
                plsize -= plsize_1;

                buf[DESCRIPTOR_LENGTH_IDX] = plsize_1 + 14;
                buf[PRIVDATA_LENGTH_IDX]   = plsize_1 + 3;
                util::BYTE combine;
                combine = (seqNumber << 1) | (plsize > 0 ? 1 : 0);
                ++seqNumber;
                buf[SEQNUM_IDX] = combine;

                memcpy(payload_ptr, payload, plsize_1);
                payload_ptr[plsize_1] = '\0'; // FCS Checksum?

                MAKE_MSG(buf, messages::editingCommand, PAYLOAD_IDX+plsize_1+1);
                util::Buffer buffer( buf, PAYLOAD_IDX+plsize_1+1, false );
                conn->send(&buffer);
                payload += plsize_1;
        }
}


void EditingCommandHandler::send(Connector* conn, const util::Buffer& descriptorSection)
{
        util::Buffer connBuffer;
        int         descriptorLength = descriptorSection.length();
        const char* descriptorData   = descriptorSection.buffer();

        connBuffer.copy(3, descriptorData, descriptorLength);
        MAKE_MSG (connBuffer.buffer(), messages::editingCommand, descriptorLength+3);
        conn->send( &connBuffer );
}

void EditingCommandHandler::process( util::Buffer *msg )
{
        util::WORD  msgSize = MSG_PAYLOAD_SIZE( (unsigned char*) msg->buffer());
        util::BYTE *payload = MSG_PAYLOAD(msg->buffer());
        int offset = 2;

        if (msgSize < 16) {
            printf( "[EditingCommandHandler] Invalid size for an editingCommand\n");
            return;
        }

        util::WORD  eventID = RW(payload, offset);
        util::QWORD npt     = RQW(payload, offset);
        npt &= 0x1FFFFFFFFULL;

        ++offset; // privateDataLength
        util::BYTE commandTag     = RB(payload, offset);
        util::BYTE combine        = RB(payload, offset);
        util::BYTE sequenceNumber = combine >> 1;
        bool       finalFlag      = combine & 0x01 ? true : false;

        EditingCommandData* ecd = new EditingCommandData(commandTag);
        ecd->setNPT(npt);
        ecd->setEventID(eventID);
        ecd->setFinal(finalFlag);
        ecd->setSequenceNumber(sequenceNumber);
        ecd->addPayload(payload+offset, msgSize-offset-1);

        // This is a continuation of previously sent editing command pieces...
        if (sequenceNumber > 0) {
                if (_incompleteEditingCommands.count(commandTag) != 0) {
                        EditingCommandData* ecdo = _incompleteEditingCommands[commandTag];

                        if (ecdo->getSequenceNumber() == sequenceNumber-1) {
                                ecdo->addPayload(ecd->getPayload(), ecd->getPayloadSize());
                                ecdo->setSequenceNumber(sequenceNumber);
                                ecdo->setFinal(finalFlag);
                                delete ecd;
                                ecd = ecdo;
                        } else {
                                // Somethings wrong, unexpected SeqNumber!
                                delete ecd;
                                delete ecdo;
                                _incompleteEditingCommands.erase(commandTag);
                                return;
                        }
                } else {
                        // Somethings wrong, unexpected SeqNumber!
                        delete ecd;
                        return;
                }
        } else {
                if (_incompleteEditingCommands.count(commandTag) != 0) {
                        // Previously sent EC didn't complete...
                        delete _incompleteEditingCommands[commandTag];
                }
                _incompleteEditingCommands[commandTag] = ecd;
        }

        // FinalFlag == 0 when this is the last part of an editing command.
        if (finalFlag == 0) {

            EditingCommandData* ecdo = _incompleteEditingCommands[commandTag];
            _incompleteEditingCommands.erase(commandTag);
            _onEditingCommandHndl(ecdo);
	    delete ecdo;
        }
}

void EditingCommandHandler::onEditingCommandHandler(const EditingCommandCallbackT &callback){
    _onEditingCommandHndl = callback;
}

int EditingCommandHandler::getIncompleteEditingCommandsCount()
{
    return (int)_incompleteEditingCommands.size();
}

EditingCommandData::EditingCommandData()
{
	privateDataPayload = new util::Buffer(MAX_EDITINGCOMMAND_SIZE);
}

EditingCommandData::EditingCommandData (unsigned char commandTag)
: eventID(0)
, eventNPT(0)
, commandTag(commandTag)
, sequenceNumber(0)
, finalFlag(false)
{
	privateDataPayload = new util::Buffer(MAX_EDITINGCOMMAND_SIZE);
}


void EditingCommandData::copyFrom(EditingCommandData* ecd){
    eventID = ecd->getEventID();
    eventNPT = ecd->getNPT();
    commandTag = ecd->getCommandTag();
    sequenceNumber = ecd->getSequenceNumber();
    finalFlag = ecd->isFinal();

    privateDataPayload->append((const char*)ecd->getPayload(), ecd->getPayloadSize());
}

EditingCommandData::~EditingCommandData()
{
    delete privateDataPayload;
}

void EditingCommandData::setFinal(bool finalFlag)
{
        this->finalFlag = finalFlag;
}
void EditingCommandData::setNPT(unsigned long long npt)
{
        this->eventNPT = npt;
}

void EditingCommandData::setEventID(unsigned long eid)
{
        this->eventID = eid;
}

void EditingCommandData::setSequenceNumber(unsigned char snmbr)
{
        this->sequenceNumber = snmbr;
}

void EditingCommandData::setPrivateDataPayload(const unsigned char* payload, const int payloadSize)
{
	privateDataPayload->copy((const char*)payload,payloadSize);
}

void EditingCommandData::addPayload(const unsigned char* buffer, int length)
{
	privateDataPayload->append((const char*)buffer, length);
}

void EditingCommandData::payloadCompleted()
{
        this->parsePayloadImpl();
}


bool EditingCommandData::isFinal() const
{
        return finalFlag;
}

unsigned long long EditingCommandData::getNPT() const
{
        return eventNPT;
}

unsigned long EditingCommandData::getEventID() const
{
        return eventID;
}

unsigned char EditingCommandData::getCommandTag() const
{
        return commandTag;
}

unsigned char EditingCommandData::getSequenceNumber() const
{
        return sequenceNumber;
}

int EditingCommandData::getPayloadSize() const
{
	return privateDataPayload->length();
        //return privateDataPayloadSize;
}

const unsigned char* EditingCommandData::getPayload() const
{
	return (const unsigned char*)privateDataPayload->buffer();
        //return privateDataPayload;
}

} // namespace connector

