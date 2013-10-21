/**
 * xrdp: A Remote Desktop Protocol server.
 *
 * Copyright (C) Jay Sorg 2004-2013
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * module manager
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xrdp.h"

#include <pwd.h>
#include <grp.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/shm.h>
#include <sys/stat.h>

#include <sys/ioctl.h>
#include <sys/socket.h>

#include <winpr/crt.h>
#include <winpr/pipe.h>
#include <winpr/path.h>
#include <winpr/synch.h>
#include <winpr/thread.h>
#include <winpr/stream.h>
#include <winpr/sspicli.h>
#include <winpr/environment.h>

#include <freerdp/freerdp.h>

#include "../module/X11/x11_module.h"
#include "../module/RDP/rdp_module.h"
#include "../module/CEF/cef_module.h"

extern char* RdsModuleName;

int xrdp_client_get_event_handles(rdsModule* module, HANDLE* events, DWORD* nCount)
{
	rdsConnector* connector;

	connector = (rdsConnector*) module;

	if (module)
	{
		if (connector->ServerQueue)
		{
			events[*nCount] = MessageQueue_Event(connector->ServerQueue);
			(*nCount)++;
		}
	}

	return 0;
}

int xrdp_client_check_event_handles(rdsModule* module)
{
	int status = 0;
	rdsConnector* connector;

	connector = (rdsConnector*) module;

	if (!module)
		return 0;

	while (WaitForSingleObject(MessageQueue_Event(connector->ServerQueue), 0) == WAIT_OBJECT_0)
	{
		status = xrdp_message_server_queue_process_pending_messages(module);
	}

	return status;
}

rdsModule* xrdp_module_new(rdsSession* session)
{
	int error_code;
	int auth_status;
	rdsModule* module;
	rdpSettings* settings;
	rdsConnector* connector;
	RDS_MODULE_ENTRY_POINTS EntryPoints;

	ZeroMemory(&EntryPoints, sizeof(EntryPoints));
	EntryPoints.Version = RDS_MODULE_INTERFACE_VERSION;
	EntryPoints.Size = sizeof(EntryPoints);

	if (!RdsModuleName)
		RdsModuleName = _strdup("X11");

	if (strcmp(RdsModuleName, "RDP") == 0)
		RDP_RdsModuleEntry(&EntryPoints);
	else if (strcmp(RdsModuleName, "X11") == 0)
		X11_RdsModuleEntry(&EntryPoints);
	else if (strcmp(RdsModuleName, "CEF") == 0)
		CEF_RdsModuleEntry(&EntryPoints);

	settings = session->settings;

	auth_status = xrdp_authenticate(settings->Username, settings->Password, &error_code);

	module = (rdsModule*) malloc(EntryPoints.ContextSize);
	ZeroMemory(module, EntryPoints.ContextSize);

	connector = (rdsConnector*) module;

	module->Size = EntryPoints.ContextSize;
	module->SessionId = 10;

	connector->session = session;
	connector->settings = session->settings;

	connector->pEntryPoints = (RDS_MODULE_ENTRY_POINTS*) malloc(sizeof(RDS_MODULE_ENTRY_POINTS));
	CopyMemory(connector->pEntryPoints, &EntryPoints, sizeof(RDS_MODULE_ENTRY_POINTS));

	connector->GetEventHandles = xrdp_client_get_event_handles;
	connector->CheckEventHandles = xrdp_client_check_event_handles;

	module->client = freerds_client_outbound_interface_new();
	module->server = freerds_server_outbound_interface_new();

	module->OutboundStream = Stream_New(NULL, 8192);
	module->InboundStream = Stream_New(NULL, 8192);

	module->InboundTotalLength = 0;
	module->InboundTotalCount = 0;

	module->OutboundTotalLength = 0;
	module->OutboundTotalCount = 0;

	connector->StopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	connector->pEntryPoints->New((rdsModule*) module);

	connector->ServerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) xrdp_client_thread,
			(void*) module, CREATE_SUSPENDED, NULL);

	freerds_client_inbound_module_init(module);

	connector->pEntryPoints->Start(module);

	ResumeThread(connector->ServerThread);

	return module;
}

void xrdp_module_free(rdsModule* module)
{
	rdsConnector* connector;

	connector = (rdsConnector*) module;

	SetEvent(connector->StopEvent);

	WaitForSingleObject(connector->ServerThread, INFINITE);
	CloseHandle(connector->ServerThread);

	Stream_Free(module->OutboundStream, TRUE);
	Stream_Free(module->InboundStream, TRUE);

	CloseHandle(connector->StopEvent);
	CloseHandle(module->hClientPipe);

	connector->pEntryPoints->Free(module);

	free(module);
}
