#include <iostream>

#include <slang.h>
#include <slang-gfx.h>
#include <slang-com-helper.h>
//#include "slang-test-tool-util.h"

void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob)
{
    if (diagnosticsBlob != nullptr)
    {
        printf("%s", (const char*)diagnosticsBlob->getBufferPointer());
    }
}

//gfx::Result 
//loadShaderProgram(gfx::IDevice* device, gfx::IShaderProgram** outProgram)
//{
//    gfx::ComPtr<slang::ISession> slangSession;
//    slangSession = device->getSlangSession();
//
//    gfx::ComPtr<slang::IBlob> diagnosticsBlob;
//    slang::IModule* module = slangSession->loadModule("diff_func.slang", diagnosticsBlob.writeRef());
//    diagnoseIfNeeded(diagnosticsBlob);
//    
//    if (!module)
//        return SLANG_FAIL;
//
//    gfx::ComPtr<slang::IEntryPoint> entryPoint;
//    SLANG_RETURN_ON_FAIL(module->findEntryPointByName("computeMain", entryPoint.writeRef()));
//
//    Slang::List<slang::IComponentType*> componentTypes;
//    componentTypes.add(module);
//
//    int entryPointCount = 0;
//    int funcEntryPointIndex = entryPointCount++;
//    
//    componentTypes.add(entryPoint);
//
//    gfx::ComPtr<slang::IComponentType> linkedProgram;
//    SlangResult result = slangSession->createCompositeComponentType(
//        componentTypes.getBuffer(),
//        componentTypes.getCount(),
//        linkedProgram.writeRef(),
//        diagnosticsBlob.writeRef());
//    diagnoseIfNeeded(diagnosticsBlob);
//    SLANG_RETURN_ON_FAIL(result);
//
//    gfx::IShaderProgram::Desc programDesc = {};
//    programDesc.slangGlobalScope = linkedProgram;
//    SLANG_RETURN_ON_FAIL(device->createProgram(programDesc, outProgram));
//
//    return SLANG_OK;
//}
//
//gfx::IDevice* device = nullptr;
//
//void initGfx()
//{
//    gfx::IDevice::Desc deviceDesc = {};
//    gfxCreateDevice(&deviceDesc, &device);
//}

int
main()
{
    gfx::ComPtr<slang::IGlobalSession> slangSession;
    slangSession.attach(spCreateSession(NULL));
    
    Slang::ComPtr<slang::ICompileRequest> request;
    SLANG_RETURN_ON_FAIL(slangSession->createCompileRequest(request.writeRef()));

    const int targetIndex = request->addCodeGenTarget(SLANG_SHADER_HOST_CALLABLE);
    request->setTargetFlags(targetIndex, SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM);
    const int translationUnitIndex = request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
    request->addTranslationUnitSourceFile(translationUnitIndex, "diff_func.slang");
    const SlangResult compileRes = request->compile();

    if (auto diagnostics = request->getDiagnosticOutput())
    {
        printf("%s", diagnostics);
    }

    gfx::ComPtr<ISlangSharedLibrary> sharedLibrary;
    SLANG_RETURN_ON_FAIL(request->getTargetHostCallable(0, sharedLibrary.writeRef()));

    {
        float **J = new float*[3];
        for (int i = 0; i < 3; ++i) {
            J[i] = new float[2];
        }
        typedef void (*Func)(float **);
        Func func = (Func)sharedLibrary->findFuncByName("computeMain");

        if (!func) {
            printf("Nooooo\n");
        }

        func(J);
        
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 2; ++j) {
                std::cout << J[i][j] << " ";
            }

            std::cout << std::endl;
        }
    }

    /*spDestroyCompileRequest(request);
    spDestroySession(slangSession);*/

    return 0;
}