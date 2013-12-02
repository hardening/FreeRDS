/**
 * FreeRDS: FreeRDP Remote Desktop Services (RDS)
 * NetSurf Server Module
 *
 * Copyright 2013 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 * Copyright 2013 Thinstuff Technologies GmbH
 * Copyright 2013 DI (FH) Martin Haimberger <martin.haimberger@thinstuff.at>
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
#include <winpr/synch.h>
#include <winpr/thread.h>
#include <winpr/pipe.h>
#include <winpr/environment.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <freerds/module.h>
#include <freerds/backend.h>

#include "netsurf_module.h"

RDS_MODULE_CONFIG_CALLBACKS gConfig;
RDS_MODULE_STATUS_CALLBACKS gStatus;

struct rds_module_netsurf
{
	RDS_MODULE_COMMON commonModule;

	wLog* log;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
};
typedef struct rds_module_netsurf rdsModuleNetSurf;

RDS_MODULE_COMMON* netsurf_rds_module_new(void)
{
	rdsModuleNetSurf* ns = (rdsModuleNetSurf*) malloc(sizeof(rdsModuleNetSurf));

	WLog_Init();

	ns->log = WLog_Get("com.freerds.module.netsurf");
	WLog_OpenAppender(ns->log);

	WLog_SetLogLevel(ns->log, WLOG_DEBUG);

	WLog_Print(ns->log, WLOG_DEBUG, "RdsModuleNew");

	return (RDS_MODULE_COMMON*) ns;
}

void netsurf_rds_module_free(RDS_MODULE_COMMON * module)
{
	rdsModuleNetSurf* ns = (rdsModuleNetSurf*) module;
	WLog_Print(ns->log, WLOG_DEBUG, "RdsModuleFree");
	WLog_Uninit();
	free(module);
}

void initMaxMinRes(rdsModuleNetSurf * ns) {
	char tempstr[256];

	long maxXRes, maxYRes, minXRes, minYRes = 0;

	if (!gConfig.getPropertyNumber(ns->commonModule.sessionId, "module.netsurf.maxXRes", &maxXRes)) {
		WLog_Print(ns->log, WLOG_ERROR, "Setting: module.netsurf.maxXRes not defined, NOT setting FREERDS_SMAX or FREERDS_SMIN\n");
	}
	if (!gConfig.getPropertyNumber(ns->commonModule.sessionId, "module.netsurf.maxYRes", &maxYRes)) {
		WLog_Print(ns->log, WLOG_ERROR, "Setting: module.netsurf.maxYRes not defined, NOT setting FREERDS_SMAX or FREERDS_SMIN\n");
	}
	if (!gConfig.getPropertyNumber(ns->commonModule.sessionId, "module.netsurf.minXRes", &minXRes)) {
		WLog_Print(ns->log, WLOG_ERROR, "Setting: module.netsurf.minXRes not defined, NOT setting FREERDS_SMAX or FREERDS_SMIN\n");
	}
	if (!gConfig.getPropertyNumber(ns->commonModule.sessionId, "module.netsurf.minYRes", &minYRes)){
		WLog_Print(ns->log, WLOG_ERROR, "Setting: module.netsurf.minYRes not defined, NOT setting FREERDS_SMAX or FREERDS_SMIN\n");
	}

	if ((maxXRes != 0) && (maxYRes != 0) && (minXRes != 0) && (minYRes != 0)) {
		sprintf_s(tempstr, sizeof(tempstr), "%dx%d", (unsigned int) maxXRes,(unsigned int) maxYRes );
		SetEnvironmentVariableEBA(&ns->commonModule.envBlock, "FREERDS_SMAX", tempstr);

		sprintf_s(tempstr, sizeof(tempstr), "%dx%d", (unsigned int) minXRes,(unsigned int) minYRes );
		SetEnvironmentVariableEBA(&ns->commonModule.envBlock, "FREERDS_SMIN", tempstr);
	}
}

char* netsurf_rds_module_start(RDS_MODULE_COMMON* module)
{
	BOOL status;
	char* pipeName;
	long xres, yres;
	char lpCommandLine[256];
	const char* endpoint = "NetSurf";
	rdsModuleNetSurf* ns = (rdsModuleNetSurf*) module;
	DWORD SessionId = ns->commonModule.sessionId;

	WLog_Print(ns->log, WLOG_DEBUG, "RdsModuleStart: SessionId: %d Endpoint: %s",
			(int) SessionId, endpoint);

	pipeName = (char*) malloc(256);
	freerds_named_pipe_get_endpoint_name(SessionId, endpoint, pipeName, 256);
	freerds_named_pipe_clean_endpoint(SessionId, endpoint);

	ZeroMemory(&(ns->si), sizeof(STARTUPINFO));
	ns->si.cb = sizeof(STARTUPINFO);
	ZeroMemory(&(ns->pi), sizeof(PROCESS_INFORMATION));

	if (!gConfig.getPropertyNumber(ns->commonModule.sessionId, "module.netsurf.xres", &xres))
		xres = 1024;

	if (!gConfig.getPropertyNumber(ns->commonModule.sessionId, "module.netsurf.yres", &yres))
		yres = 768;

	initMaxMinRes(ns);

	sprintf_s(lpCommandLine, sizeof(lpCommandLine), "%s -f freerds -b 32 -w %d -h %d",
			"netsurf", (int) xres, (int) yres);

	WLog_Print(ns->log, WLOG_DEBUG, "Starting process with command line: %s", lpCommandLine);

	status = CreateProcessA(NULL, lpCommandLine,
			NULL, NULL, FALSE, 0, ns->commonModule.envBlock, NULL,
			&(ns->si), &(ns->pi));

	WLog_Print(ns->log, WLOG_DEBUG, "Process created with status: %d", status);

	if (!WaitNamedPipeA(pipeName, 5 * 1000))
	{
		fprintf(stderr, "WaitNamedPipe failure: %s\n", pipeName);
		return NULL;
	}

	return pipeName;
}

int netsurf_rds_module_stop(RDS_MODULE_COMMON* module)
{
	rdsModuleNetSurf* ns = (rdsModuleNetSurf*) module;

	WLog_Print(ns->log, WLOG_DEBUG, "RdsModuleStop");

	return 0;
}

int RdsModuleEntry(RDS_MODULE_ENTRY_POINTS* pEntryPoints)
{
	pEntryPoints->Version = 1;
	pEntryPoints->Name = "NetSurf";

	pEntryPoints->New = netsurf_rds_module_new;
	pEntryPoints->Free = netsurf_rds_module_free;

	pEntryPoints->Start = netsurf_rds_module_start;
	pEntryPoints->Stop = netsurf_rds_module_stop;

	gStatus = pEntryPoints->status;
	gConfig = pEntryPoints->config;

	return 0;
}

