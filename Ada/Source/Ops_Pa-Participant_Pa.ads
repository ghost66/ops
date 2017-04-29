--
-- Copyright (C) 2016 Lennart Andersson.
--
-- This file is part of OPS (Open Publish Subscribe).
--
-- OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Lesser General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public License
-- along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.

with Ops_Pa.Transport_Pa.SendDataHandler_Pa,
     Ops_Pa.Transport_Pa.SendDataHandlerFactory_Pa,
     Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa,
     Ops_Pa.Transport_Pa.ReceiveDataHandlerFactory_Pa,
     Ops_Pa.SerializableFactory_Pa,
     Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.OpsObjectFactory_Pa,
     Ops_Pa.Error_Pa,
     Ops_Pa.OpsObject_Pa.OPSConfig_Pa,
     Ops_Pa.OpsObject_Pa.Domain_Pa,
     Ops_Pa.OpsObject_Pa.Topic_Pa;

use Ops_Pa.Transport_Pa.SendDataHandler_Pa,
    Ops_Pa.Transport_Pa.SendDataHandlerFactory_Pa,
    Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa,
    Ops_Pa.Transport_Pa.ReceiveDataHandlerFactory_Pa,
    Ops_Pa.SerializableFactory_Pa,
    Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.OpsObjectFactory_Pa,
    Ops_Pa.Error_Pa,
    Ops_Pa.OpsObject_Pa.OPSConfig_Pa,
    Ops_Pa.OpsObject_Pa.Domain_Pa,
    Ops_Pa.OpsObject_Pa.Topic_Pa;

package Ops_Pa.Participant_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type Participant_Class is new Ops_Class with private;
  type Participant_Class_At is access all Participant_Class'Class;

  -- Get a Participant instance
  function getInstance(domainID : String) return Participant_Class_At;
  function getInstance(domainID : String; participantID : String) return Participant_Class_At;
  function getInstance(domainID : String; participantID : String; configFile : String) return Participant_Class_At;

  -- NOTE only for debug.
  -- If used otherwise, you must be sure that ALL users of this participant in the
  -- program has been closed down, ortherwise the program probably will crash.
  procedure releaseInstance( part : Participant_Class_At );

  -- Add a SerializableFactory which has support for data types (i.e. OPSObject derivatives you want this Participant to understand)
  -- Takes over ownership of the object and it will be deleted with the participant
  procedure addTypeSupport( Self: in out Participant_Class; typeSupport : SerializableFactory_Class_At );

  function getTopic( Self: Participant_Class; name : string) return Topic_Class_At;

  -- Should only be used by Publishers
  function getSendDataHandler( Self: in out Participant_Class; top : Topic_Class_At) return SendDataHandler_Class_At;
  procedure releaseSendDataHandler( Self: in out Participant_Class; top : Topic_Class_At );

  -- Should only be used by Subscribers
  function getReceiveDataHandler( Self: in out Participant_Class; top : Topic_Class_At) return ReceiveDataHandler_Class_At;
  procedure releaseReceiveDataHandler( Self: in out Participant_Class; top : Topic_Class_At );

  procedure ReportError( Self : in out Participant_Class; Error : Error_Class_At );

  -- Return a reference to the internal ErrorService instance
  --
  -- Add a listener to the service to get error reports from the participant and its objects
  -- Prototype for listener is:
  --    procedure <someobject>.OnErrorReport(Sender : TObject; Error : TError);
  --
  -- Note that several threads at a time can report errors so take that into account
  function getErrorService( Self : in out Participant_Class ) return ErrorService_Class_At;

private
-- ==========================================================================
--
-- ==========================================================================
  type Participant_Class is new Ops_Class with
    record
      -- The id of this participant, must be unique in process
      ParticipantID : String_At := null;

      -- The domainID for this Participant
      DomainID : String_At := null;

      -- Objects on loan from OPSConfig
      Config : OPSConfig_Class_At := null;
      Domain : Domain_Class_At := null;

      -- The ErrorService
      ErrorService : ErrorService_Class_At := null;

      --
      ObjectFactory : OPSObjectFactory_Class_At := null;

      ------------------------------------------------------------------------
      -- The ParticipantInfoData that partInfoPub will publish periodically
      --TODO FPartInfoData : TParticipantInfoData;
      --TOOD FPartInfoDataMutex : TMutex;

      ------------------------------------------------------------------------
      --
      ReceiveDataHandlerFactory : ReceiveDataHandlerFactory_Class_At := null;
      SendDataHandlerFactory : SendDataHandlerFactory_Class_At := null;

    end record;

  function Create( domainID : String; participantID : String; configFile : String ) return Participant_Class_At;

  procedure InitInstance( Self : in out Participant_Class;
                          domainID : String;
                          participantID : String;
                          configFile : String);

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  procedure Finalize( Self : in out Participant_Class );

end Ops_Pa.Participant_Pa;