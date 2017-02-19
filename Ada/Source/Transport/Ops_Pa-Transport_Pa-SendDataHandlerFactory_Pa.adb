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

with Ops_Pa.Transport_Pa.SendDataHandler_Pa.Mc_Pa;

--TODO
--     uOps.Transport.McUdpSendDataHandler,
--     uOps.Transport.TCPSendDataHandler;

package body Ops_Pa.Transport_Pa.SendDataHandlerFactory_Pa is

  use type MyMap.cursor;


  -- Constructors
  function Create(Dom : Domain_Class_At;
                  --TODO Proc : TOnUdpConnectDisconnectProc;
                  Reporter : ErrorService_Class_At) return SendDataHandlerFactory_Class_At is
     Self : SendDataHandlerFactory_Class_At := null;
  begin
    Self := new SendDataHandlerFactory_Class;
    InitInstance( Self.all, Dom, Reporter );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  function Less (Left, Right : String_At) return Boolean is
  begin
    return Left.all < Right.all;
  end;

  function Equal (Left, Right : HandlerInfo) return Boolean is
  begin
    return Left.handler = Right.handler and Left.numUsers = Right.numUsers;
  end;

  -- Generate the key used in the dictionary
  function getKey(top : Topic_Class_At) return String is
  begin
    return top.Transport & "::" & top.DomainAddress & "::" & Int32'Image(top.Port);
  end;

  function getSendDataHandler( Self : in out SendDataHandlerFactory_Class; top : Topic_Class_At) return SendDataHandler_Class_At is

    procedure Inc( key : in String_At; element : in out HandlerInfo ) is
    begin
      element.numUsers := element.numUsers + 1;
    end;

    localIf : string := doSubnetTranslation(top.LocalInterface);
    ttl : Integer := Integer(top.TimeToLive);
    result : SendDataHandler_Class_At := null;
    pos : MyMap.Cursor;
    info : HandlerInfo;

    -- We need to store SendDataHandlers with more than just the name as key.
    -- Since topics can use the same port, we need to return the same SendDataHandler.
    -- Make a key with the transport info that uniquely defines the receiver.
    key : String_At := new String'(getKey(top));

  begin
    Self.Mutex.Acquire;

    pos := Self.SendDataHandlers.Find( key );

    if pos /= MyMap.No_Element then
      -- Increment usage count
      Self.SendDataHandlers.Update_Element(pos, Process => Inc'Access);
      -- Return found handler
      info := MyMap.Element(pos);
      Self.Mutex.Release;
      return info.handler;
    end if;

    if top.Transport = TRANSPORT_MC then
      Result := SendDataHandler_Class_At(Ops_Pa.Transport_Pa.SendDataHandler_Pa.Mc_Pa.Create(top, localIf, ttl, Self.ErrorService));
      info.handler := Result;
      info.numUsers := 1;
      Self.SendDataHandlers.Insert(key, info);

    elsif top.Transport = TRANSPORT_UDP then
--///TODO      if not Assigned(FUdpSendDataHandler) then
        -- We have only one sender for all topics, so use the domain value for buffer size
--        FUdpSendDataHandler := TMcUdpSendDataHandler.Create(localIf,
--                                 ttl,
--                                 FDomain.OutSocketBufferSize,
--                                 FErrorService);
--      end if;

      -- Setup a listener on the participant info data published by participants on our domain.
      -- We use the information for topics with UDP as transport, to know the destination for UDP sends
      -- ie. we extract ip and port from the information and add it to our McUdpSendDataHandler
--///TODO      if Assigned(FOnUdpConnectDisconnectProc) then FOnUdpConnectDisconnectProc(top, FUdpSendDataHandler, True); end if;

      Self.UdpUsers := Self.UdpUsers + 1;

--///TODO      Result := FUdpSendDataHandler;
      raise Not_Yet_Implemented;

    elsif top.Transport = TRANSPORT_TCP then
--///TODO      Result := TTCPSendDataHandler.Create(top, FErrorService);
--      info.handler := Result;
      info.numUsers := 1;
      Self.SendDataHandlers.Insert(key, info);

      raise Not_Yet_Implemented;
    end if;

    Self.Mutex.Release;

    return result;

  exception
    when others =>
      Self.Mutex.Release;
      raise;
  end;

  procedure releaseSendDataHandler( Self : in out SendDataHandlerFactory_Class; top : Topic_Class_At) is

    procedure Dec( key : in String_At; element : in out HandlerInfo ) is
    begin
      element.numUsers := element.numUsers - 1;
    end;

    key : String_At := new String'(getKey(top));
    pos : MyMap.Cursor;
    info : HandlerInfo;

  begin
    Self.Mutex.Acquire;

    if top.Transport = TRANSPORT_UDP then
--///TODO      if Assigned(FOnUdpConnectDisconnectProc) then FOnUdpConnectDisconnectProc(top, FUdpSendDataHandler, False); end if;
      Self.UdpUsers := Self.UdpUsers - 1;
      if Self.UdpUsers = 0 then
        -- This is the last matching call, so now no one is using the SendDataHandler
        -- Delete it
--///TODO        FreeAndNil(FUdpSendDataHandler);
        null;
      end if;

    else
      pos := Self.SendDataHandlers.Find( key );

      if pos /= MyMap.No_Element then
        -- Decrement usage count
        Self.SendDataHandlers.Update_Element(pos, Dec'Access);

        if MyMap.Element(pos).numUsers = 0 then
          -- This is the last matching call, so now no one is using the SendDataHandler
          -- Remove it from the dictionary and then delete it
          info := MyMap.Element(pos);
          Self.SendDataHandlers.Delete(pos);
          Free(info.handler);
        end if;
      end if;
    end if;

    Self.Mutex.Release;

  exception
    when others =>
      Self.Mutex.Release;
      raise;
  end;

  procedure InitInstance( Self : in out SendDataHandlerFactory_Class;
                          Dom : Domain_Class_At;
                          --TODO Proc : TOnUdpConnectDisconnectProc;
                          Reporter : ErrorService_Class_At) is
  begin
    Self.ErrorService := Reporter;
    Self.Domain := dom;
--TODO    FOnUdpConnectDisconnectProc := Proc;
  end;

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  procedure Finalize( Self : in out SendDataHandlerFactory_Class ) is

    handlerExist : Boolean := False;

    procedure Process( Pos : in MyMap.Cursor ) is
      Value : HandlerInfo := MyMap.Element(Pos);
    begin
      if Value.numUsers /= 0 then
        handlerExist := True;
      end if;
      if Value.handler /= null then
        Free(Value.handler);
      end if;
    end;

  begin
    -- Cleanup/Free all senddatahandlers under protection
    Self.Mutex.Acquire;
    begin
      if Self.UdpUsers /= 0 then
        handlerExist := True;
      end if;

--///TODO      FreeAndNil(FUdpSendDataHandler);

      Self.SendDataHandlers.Iterate(Process'Access);

      Self.Mutex.Release;

    exception
      when others =>
        Self.Mutex.Release;
        raise;
    end;

    if handlerExist then
      if Self.ErrorService /= null then
        Self.ErrorService.Report(Error_Class_At(Create(
          "SendDataHandlerFactory", "Finalize", "Publishers still alive when deleting factory!!!")));
      end if;
    end if;
  end;

end Ops_Pa.Transport_Pa.SendDataHandlerFactory_Pa;

