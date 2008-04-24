// Jacobi.Vst.Interop.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "PluginCommandProxy.h"
#include<vcclr.h>

// fwd refs
System::String^ getPluginFileName();
AEffect* CreateAudioEffectInfo(Jacobi::Vst::Core::VstPluginInfo^ pluginInfo);

// global reference to the plugin cmd stub
gcroot<PluginCommandProxy^> _pluginCommandProxy;

// main experted method called by host to create the plugin
AEffect* VSTMain (audioMasterCallback hostCallback)
{
	try
	{
		// retrieve the current plugin file name
		System::String^ interopAssemblyFileName = getPluginFileName();
		
		// create the plugin (command stub) factory
		Jacobi::Vst::Core::ManagedPluginFactory^ factory = gcnew Jacobi::Vst::Core::ManagedPluginFactory(interopAssemblyFileName);
		
		// create the managed type that implements the Plugin Command Stub interface
		Jacobi::Vst::Core::IVstPluginCommandStub^ commandStub = factory->CreatePluginCommandStub();
		
		if(commandStub)
		{
			// connect the plugin command stub to the command proxy
			_pluginCommandProxy = gcnew PluginCommandProxy(commandStub);

			// retrieve the plugin info
			Jacobi::Vst::Core::VstPluginInfo^ pluginInfo = commandStub->GetPluginInfo();

			if(pluginInfo)
			{
				// create the native audio effect struct based on the plugin info
				AEffect* pEffect = CreateAudioEffectInfo(pluginInfo);

				return pEffect;
			}
		}
	}
	catch(System::Exception^ exc)
	{
		//System::Diagnostics::Trace::WriteLine(e->ToString(), "VST.NET");
	}

	return NULL;
}

VstIntPtr DispatcherProc(AEffect* pluginInfo, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
	return _pluginCommandProxy->Dispatch(opcode, index, value, prt, opt);
}

void Process32Proc(AEffect* pluginInfo, float** inputs, float** outputs, VstInt32 sampleFrames)
{
	_pluginCommandProxy->Process(inputs, outputs, sampleFrames);
}

void Process64Proc(AEffect* pluginInfo, double** inputs, double** outputs, VstInt32 sampleFrames)
{
	_pluginCommandProxy->Process(inputs, outputs, sampleFrames);
}

void SetParameterProc(AEffect* pluginInfo, VstInt32 index, float value)
{
	_pluginCommandProxy->SetParameter(index, value);
}

float GetParameterProc(AEffect* pluginInfo, VstInt32 index)
{
	return _pluginCommandProxy->GetParameter(index);
}

AEffect* CreateAudioEffectInfo(Jacobi::Vst::Core::VstPluginInfo^ pluginInfo)
{
	AEffect* pEffect = new AEffect();
	pEffect->magic = kEffectMagic;

	// assign function pointers
	pEffect->dispatcher = ::DispatcherProc;
	pEffect->processReplacing = ::Process32Proc;
	pEffect->processDoubleReplacing = ::Process64Proc;
	pEffect->setParameter = ::SetParameterProc;
	pEffect->getParameter = ::GetParameterProc;

	// assign info data
	pEffect->flags = (int)pluginInfo->Flags;
	pEffect->initialDelay = pluginInfo->InitialDelay;
	pEffect->numInputs = pluginInfo->NumberOfAudioInputs;
	pEffect->numOutputs = pluginInfo->NumberOfAudioOutputs;
	pEffect->numParams = pluginInfo->NumberOfParameters;
	pEffect->numPrograms = pluginInfo->NumberOfPrograms;
	pEffect->uniqueID = pluginInfo->PluginID;
	pEffect->version = pluginInfo->PluginVersion;

	return pEffect;
}

System::String^ getPluginFileName()
{
	System::String^ pathAsUri = System::Reflection::Assembly::GetExecutingAssembly()->CodeBase;
	System::Uri^ uri = gcnew System::Uri(pathAsUri);
	return uri->AbsolutePath;
}