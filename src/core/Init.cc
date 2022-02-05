#include <sbl/core/Init.h>
#include <sbl/core/Command.h>
#include <sbl/core/File.h>
#include <sbl/core/StringUtil.h>
#include <sbl/core/UnitTest.h>
#include <sbl/math/VectorUtil.h>
#include <sbl/math/OptimizerUtil.h>
#include <sbl/system/Signal.h>
#include <sbl/image/ImageUtil.h>
#include <sbl/other/CodeCheck.h>
#ifdef USE_PYTHON
	#include <sbl/other/Scripting.h>
#endif
namespace sbl {


/// register all SBL commands, etc.
void initModules() {

	// core modules
	initCommand();
	initFile();
	initStringUtil();
	initUnitTest();

	// math modules
	initVectorUtil();
	initOptimizerUtil();

	// system modules
	initSignal();

	// image modules
	initImageUtil();

	// other modules
	initCodeCheck();
#ifdef USE_PYTHON
	initScripting();
#endif
}


} // end namespace sbl
