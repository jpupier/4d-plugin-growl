/* --------------------------------------------------------------------------------
 #
 #	4DPlugin.c
 #	source generated by 4D Plugin Wizard
 #	Project : Growl II
 #	author : miyako
 #	2013/10/01
 #
 # --------------------------------------------------------------------------------*/


#include "4DPlugin.h"


namespace Growl
{
    GrowlListener *listener = nil;
};

void PluginMain(int32_t selector, PA_PluginParameters params)
{
	try
	{
		int32_t pProcNum = selector;
		sLONG_PTR *pResult = (sLONG_PTR *)params->fResult;
		PackagePtr pParams = (PackagePtr)params->fParameters;

		CommandDispatcher(pProcNum, pResult, pParams); 
	}
	catch(...)
	{

	}
}

void CommandDispatcher (int32_t pProcNum, sLONG_PTR *pResult, PackagePtr pParams)
{
	switch(pProcNum)
	{
		case kDeinitPlugin:
		case kServerDeinitPlugin:
			listenerLoopFinish();
			break;			
			
		case kInitPlugin:
		case kServerInitPlugin:
			break;	
			
// --- 1.3 SDK

		case 1 :
			Growl_Set_notification_method(pResult, pParams);
			break;

		case 2 :
			Growl_SET_MIST_ENABLED(pResult, pParams);
			break;

		case 3 :
			Growl_Get_mist_enabled(pResult, pParams);
			break;

		case 4 :
			Growl_POST_NOTIFICATION(pResult, pParams);
			break;

		case 5 :
			Growl_Get_notification_method(pResult, pParams);
			break;

	}
}

#pragma mark -

void listenerLoopExecuteMethodByID()
{
	PA_Variable	params[2];
	params[0] = PA_CreateVariable(eVK_Unistring);		
	params[1] = PA_CreateVariable(eVK_Unistring);	
	
	PA_Unistring u1 = PA_setUnistringVariable(&params[0], Growl::listener.notificationType);
	PA_Unistring u2 = PA_setUnistringVariable(&params[1], Growl::listener.notificationName);			
	
	PA_ExecuteMethodByID([Growl::listener.listenerMethodId intValue], params, 2);
	
	[Growl::listener unlock];
	
	PA_DisposeUnistring(&u1);
	PA_DisposeUnistring(&u2);
}

void listenerLoopFinish(){
	
	if(Growl::listener){
		[[NSNotificationCenter defaultCenter]removeObserver:Growl::listener];
		[Growl::listener release];
		Growl::listener = nil;
	}	
	
}

void listenerLoop(){
	
	PA_YieldAbsolute();	
	int listenerProcessNumber = PA_GetCurrentProcessNumber();
	PA_FreezeProcess(listenerProcessNumber);
	
	char done = 0;
	
	while (!done)
	{ 
		PA_YieldAbsolute();
		done = (PA_IsProcessDying()) || ([Growl::listener shouldTerminate]);
		
		if (!done){		
			
			PA_NewProcess((void *)listenerLoopExecuteMethodByID, 512*1024, Growl::listener.notificationType);
			
			PA_FreezeProcess(listenerProcessNumber);
			
		}
	}
	
	listenerLoopFinish();
	
	PA_KillProcess();
}

#pragma mark -

// ------------------------------------ 1.3 SDK -----------------------------------

void Growl_Set_notification_method(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_TEXT Param1;
	C_LONGINT returnValue;
	
	Param1.fromParamAtIndex(pParams, 1);
	
	int success = 0;
	
	NSString *methodName = Param1.copyUTF16String();
	
	if(!Param1.getUTF16Length()){
		
		if(Growl::listener)
		{
			Growl::listener.listenerMethodName = @"";
			Growl::listener.listenerMethodId = [NSNumber numberWithInt:0];	
			
			success = 1;
		}
		
	}else{
		
		int methodId = PA_GetMethodID((PA_Unichar *)Param1.getUTF16StringPtr());
		
		if(methodId){
			
			success = 1;
			
			if(!Growl::listener){	
				
				int GrowlListenerProcessNumber = PA_NewProcess((void *)listenerLoop, -512*1024, @"$Growl Notification Process");
				
				Growl::listener = [[GrowlListener alloc]
							initWithMethodName:methodName
							methodId:[NSNumber numberWithInt:methodId]
							processNumber:[NSNumber numberWithInt:GrowlListenerProcessNumber]];
				
			}else{
				Growl::listener.listenerMethodName = methodName;
				Growl::listener.listenerMethodId = [NSNumber numberWithInt:methodId];				
			}
			
		}
		
	}
	[methodName release];
	
	returnValue.setIntValue(success);
	returnValue.setReturn(pResult);
}

void Growl_SET_MIST_ENABLED(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT Param1;
	
	Param1.fromParamAtIndex(pParams, 1);
	
	[GrowlApplicationBridge setShouldUseBuiltInNotifications:Param1.getIntValue()];
	
}

void Growl_Get_mist_enabled(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT returnValue;
	
	returnValue.setIntValue([GrowlApplicationBridge shouldUseBuiltInNotifications]);
	
	returnValue.setReturn(pResult);
}

void Growl_POST_NOTIFICATION(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_TEXT ParamTitle;
	C_TEXT ParamDescription;
	C_TEXT ParamNotificationName;
	ParamTitle.fromParamAtIndex(pParams, 1);
	ParamDescription.fromParamAtIndex(pParams, 2);
	ParamNotificationName.fromParamAtIndex(pParams, 3);
	NSString *title = ParamTitle.copyUTF16String();
	NSString *description = ParamDescription.copyUTF16String();
	NSString *notificationName = ParamNotificationName.copyUTF16String();
	
	C_BLOB ParamIconPicture;
	ParamIconPicture.fromParamAtIndex(pParams, 4);
	NSData * iconData = nil;
	if(ParamIconPicture.getBytesLength() != 0)
		iconData = [[NSData alloc]initWithBytes:ParamIconPicture.getBytesPtr() length:ParamIconPicture.getBytesLength()];
	
	C_LONGINT ParamPriority;
	C_LONGINT ParamIsSticky;
	ParamPriority.fromParamAtIndex(pParams, 5);
	ParamIsSticky.fromParamAtIndex(pParams, 6);	
	signed int priority = 0;//-2 to +2	
	BOOL isSticky = (BOOL)ParamIsSticky.getIntValue();
	
	C_TEXT ParamClickContext;
	ParamClickContext.fromParamAtIndex(pParams, 7);
	NSString *clickContext = ParamClickContext.copyUTF16String();
	
	C_TEXT ParamIdentifier;	
	ParamIdentifier.fromParamAtIndex(pParams, 8);
	NSString *identifier = nil;
	if(ParamIdentifier.getUTF16Length() != 0)
		identifier = ParamIdentifier.copyUTF16String();
	
	if(!Growl::listener){
		
		int GrowlListenerProcessNumber = PA_NewProcess((void *)listenerLoop, -512*1024, @"$Growl Notification Process");
		
		Growl::listener = [[GrowlListener alloc]
					initWithMethodName:@""
					methodId:[NSNumber numberWithInt:0]
					processNumber:[NSNumber numberWithInt:GrowlListenerProcessNumber]];
		
	}else{
		[GrowlApplicationBridge setGrowlDelegate:nil];			
	}
	
	Growl::listener.appName = @"4D";
	Growl::listener.notificationName = notificationName;
	
	[GrowlApplicationBridge setGrowlDelegate:Growl::listener];		
	
	[GrowlApplicationBridge notifyWithTitle:title 
								description:description 
						   notificationName:notificationName 
								   iconData:iconData 
								   priority:priority 
								   isSticky:isSticky 
							   clickContext:clickContext 
								 identifier:identifier];	
	
	[title release];
	[description release];
	[notificationName release];
	[clickContext release];
	
	if(iconData) [iconData release];	
	if(identifier) [identifier release];
}

void Growl_Get_notification_method(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_TEXT Param1;
	C_LONGINT returnValue;
	
	Param1.fromParamAtIndex(pParams, 1);
	
	int success = 0;
	
	if(Growl::listener)
	{
		Param1.setUTF16String(Growl::listener.listenerMethodName);
		Param1.toParamAtIndex(pParams, 1);
		success = 1;		
	}
	
	returnValue.setIntValue(success);		
	returnValue.setReturn(pResult);
}