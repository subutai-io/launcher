#ifndef __INSTALL_IMPL_H__
#define __INSTALL_IMPL_H__

#include "Vars.h"

#if LAUNCHER_WINDOWS

#include <string.h>
#include <vector>

#include "InstallImpl_Base.h"
#include "FileSystem.h"
#include "Environment.h"
#include "SubutaiException.h"
#include "SubutaiString.h"

namespace SubutaiLauncher
{
	class InstallImpl : public InstallImpl_Base
	{
	public:
		InstallImpl();
		void preInstall();
		void install();
		void postInstall();
	};
}

#endif

#endif
