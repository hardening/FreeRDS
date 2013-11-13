/**
 * FreeRDS: FreeRDP Remote Desktop Services (RDS)
 * Qt Server Module
 *
 * Copyright 2013 Marc-Andre Moreau <marcandre.moreau@gmail.com>
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
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <winpr/crt.h>
#include <winpr/wlog.h>
#include <winpr/pipe.h>
#include <winpr/synch.h>
#include <winpr/thread.h>
#include <winpr/environment.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <freerds/freerds.h>
#include <freerds/module_connector.h>

#include "qt_module.h"

pgetPropertyBool gGetPropertyBool;
pgetPropertyNumber gGetPropertyNumber;
pgetPropertyString gGetPropertyString;

struct rds_module_qt
{
	RDS_MODULE_COMMON commonModule;

	wLog* log;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
};
typedef struct rds_module_qt rdsModuleQt;

RDS_MODULE_COMMON* qt_rds_module_new(void)
{
	rdsModuleQt* qt = (rdsModuleQt*) malloc(sizeof(rdsModuleQt));

	WLog_Init();

	qt->log = WLog_Get("com.freerds.module.qt");
	WLog_OpenAppender(qt->log);

	WLog_SetLogLevel(qt->log, WLOG_DEBUG);

	WLog_Print(qt->log, WLOG_DEBUG, "RdsModuleNew");

	return (RDS_MODULE_COMMON*) qt;
}

void qt_rds_module_free(RDS_MODULE_COMMON* module)
{
	rdsModuleQt* qt = (rdsModuleQt*) module;
	WLog_Print(qt->log, WLOG_DEBUG, "RdsModuleFree");
	WLog_Uninit();
	free(module);
}

char* qt_rds_module_start(RDS_MODULE_COMMON* module)
{
	BOOL status;
	char* pipeName;
	char* qPipeName;
	long xres, yres;
	char lpCommandLine[256];
	const char* endpoint = "Qt";
	rdsModuleQt* qt = (rdsModuleQt*) module;
	DWORD SessionId = qt->commonModule.sessionId;

	WLog_Print(qt->log, WLOG_DEBUG, "RdsModuleStart: SessionId: %d Endpoint: %s",
			(int) SessionId, endpoint);

	pipeName = (char*) malloc(256);
	freerds_named_pipe_get_endpoint_name(SessionId, endpoint, pipeName, 256);
	freerds_named_pipe_clean_endpoint(SessionId, endpoint);

	ZeroMemory(&(qt->si), sizeof(STARTUPINFO));
	qt->si.cb = sizeof(STARTUPINFO);
	ZeroMemory(&(qt->pi), sizeof(PROCESS_INFORMATION));

	if (!gGetPropertyNumber(qt->commonModule.sessionId, "module.rdp.xres", &xres))
		xres = 1024;

	if (!gGetPropertyNumber(qt->commonModule.sessionId, "module.rdp.yres", &yres))
		yres = 768;

	qPipeName = (char*) malloc(256);
	sprintf_s(qPipeName, 256, "/tmp/.pipe/FreeRDS_%d_%s", (int) SessionId, endpoint);
	SetEnvironmentVariableEBA(qt->commonModule.envBlock, "FREERDS_PIPE_PATH", qPipeName);
	free(qPipeName);

	SetEnvironmentVariableEBA(qt->commonModule.envBlock, "QT_PLUGIN_PATH",
			"/opt/freerds/lib64/plugins");

	sprintf_s(lpCommandLine, sizeof(lpCommandLine), "%s -platform freerds",
			"simple_greeter");

	WLog_Print(qt->log, WLOG_DEBUG, "Starting process with command line: %s", lpCommandLine);

	status = CreateProcessA(NULL, lpCommandLine,
			NULL, NULL, FALSE, 0, *(qt->commonModule.envBlock), NULL,
			&(qt->si), &(qt->pi));

	WLog_Print(qt->log, WLOG_DEBUG, "Process created with status: %d", status);

	if (!WaitNamedPipeA(pipeName, 5 * 1000))
	{
		fprintf(stderr, "WaitNamedPipe failure: %s\n", pipeName);
		return NULL;
	}

	return pipeName;
}

int qt_rds_module_stop(RDS_MODULE_COMMON* module)
{
	rdsModuleQt* qt = (rdsModuleQt*) module;

	WLog_Print(qt->log, WLOG_DEBUG, "RdsModuleStop");

	return 0;
}

int RdsModuleEntry(RDS_MODULE_ENTRY_POINTS* pEntryPoints)
{
	pEntryPoints->Version = 1;
	pEntryPoints->Name = "Qt";

	pEntryPoints->New = qt_rds_module_new;
	pEntryPoints->Free = qt_rds_module_free;

	pEntryPoints->Start = qt_rds_module_start;
	pEntryPoints->Stop = qt_rds_module_stop;

	gGetPropertyBool = pEntryPoints->getPropertyBool;
	gGetPropertyNumber = pEntryPoints->getPropertyNumber;
	gGetPropertyString = pEntryPoints->getPropertyString;

	return 0;
}
