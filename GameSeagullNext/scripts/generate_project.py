#!/usr/bin/env python3
"""Generate an ordinary Xcode project using only Python's standard library."""
import hashlib
import pathlib
import plistlib
from xml.sax.saxutils import escape

root = pathlib.Path(__file__).resolve().parents[1]
project = root / 'GameSeagullNext.xcodeproj'
project.mkdir(exist_ok=True)
objects = {}
def uid(s): return hashlib.sha256(s.encode()).hexdigest()[:24].upper()
def quote(s): return '"'+str(s).replace('\\','\\\\').replace('"','\\"')+'"'
def obj(key, value): objects[uid(key)] = value; return uid(key)
def arr(values): return '('+','.join(values)+',)'
def settings(d): return '{'+' '.join(f'{k} = {quote(v)};' for k,v in d.items())+'}'
files = {}
allpaths = sorted([*root.glob('ios/App/*.swift'), *root.glob('ios/Shared/*.swift'), *root.glob('ios/Broadcast/*.swift'), *root.glob('ios/Shared/*.mm'), *root.glob('core/*.cpp')])
allpaths = [p for p in allpaths if p.name != 'CAPI.cpp']
for p in allpaths:
    rel = p.relative_to(root).as_posix()
    kind = {'.swift':'sourcecode.swift','.mm':'sourcecode.cpp.objcpp','.cpp':'sourcecode.cpp.cpp'}[p.suffix]
    files[rel] = obj('file:'+rel,f'{{isa = PBXFileReference; lastKnownFileType = {kind}; path = {quote(rel)}; sourceTree = "<group>";}}')
privacy = 'ios/Shared/PrivacyInfo.xcprivacy'
files[privacy] = obj('file:'+privacy, f'{{isa = PBXFileReference; lastKnownFileType = text.xml; path = {quote(privacy)}; sourceTree = "<group>";}}')
products=[]; targets=[]
base={'IPHONEOS_DEPLOYMENT_TARGET':'16.0','SDKROOT':'iphoneos','SWIFT_VERSION':'5.0','CLANG_CXX_LANGUAGE_STANDARD':'c++17','CLANG_ENABLE_MODULES':'YES','CLANG_ENABLE_OBJC_ARC':'YES','TARGETED_DEVICE_FAMILY':'1,2','CURRENT_PROJECT_VERSION':'1','MARKETING_VERSION':'0.1.0','CODE_SIGN_STYLE':'Automatic','GENERATE_INFOPLIST_FILE':'NO'}
for name,ext in [('GameSeagullNext',False),('Broadcast',True)]:
    product=obj(name+':product',f'{{isa = PBXFileReference; explicitFileType = {"wrapper.app-extension" if ext else "wrapper.application"}; path = {name}.{"appex" if ext else "app"}; sourceTree = BUILT_PRODUCTS_DIR;}}')
    products.append(product)
    sources=[r for r in files if r.endswith('.swift') and ('Shared/' in r or ('Broadcast/' if ext else 'App/') in r)]
    if ext: sources += [r for r in files if r.endswith(('.cpp','.mm'))]
    builds=[obj(name+':build:'+r,f'{{isa = PBXBuildFile; fileRef = {files[r]};}}') for r in sources]
    src=obj(name+':sources',f'{{isa = PBXSourcesBuildPhase; buildActionMask = 2147483647; files = {arr(builds)}; runOnlyForDeploymentPostprocessing = 0;}}')
    resfile=obj(name+':privacy',f'{{isa = PBXBuildFile; fileRef = {files[privacy]};}}')
    res=obj(name+':resources',f'{{isa = PBXResourcesBuildPhase; buildActionMask = 2147483647; files = {arr([resfile])}; runOnlyForDeploymentPostprocessing = 0;}}')
    fw=obj(name+':frameworks','{isa = PBXFrameworksBuildPhase; buildActionMask = 2147483647; files = (); runOnlyForDeploymentPostprocessing = 0;}')
    configs=[]
    for config in ['Debug','Release']:
        d=dict(base,PRODUCT_BUNDLE_IDENTIFIER='org.seagull.next'+('.broadcast' if ext else ''),PRODUCT_NAME='$(TARGET_NAME)',INFOPLIST_FILE='ios/'+('Broadcast' if ext else 'App')+'/Info.plist',SWIFT_OPTIMIZATION_LEVEL='-Onone' if config=='Debug' else '-O')
        if ext:d.update(APPLICATION_EXTENSION_API_ONLY='YES',SKIP_INSTALL='YES',SWIFT_OBJC_BRIDGING_HEADER='ios/Shared/SGBridge.h')
        configs.append(obj(name+':'+config,f'{{isa = XCBuildConfiguration; buildSettings = {settings(d)}; name = {config};}}'))
    cl=obj(name+':configs',f'{{isa = XCConfigurationList; buildConfigurations = {arr(configs)}; defaultConfigurationIsVisible = 0; defaultConfigurationName = Release;}}')
    phases=[src,fw,res];deps=[]
    if not ext:
        copyfile=obj('embedfile',f'{{isa = PBXBuildFile; fileRef = {uid("Broadcast:product")}; settings = {{ATTRIBUTES = (RemoveHeadersOnCopy,);}};}}')
        phases.append(obj('embed',f'{{isa = PBXCopyFilesBuildPhase; buildActionMask = 2147483647; dstPath = ""; dstSubfolderSpec = 13; files = {arr([copyfile])}; name = "Embed App Extensions"; runOnlyForDeploymentPostprocessing = 0;}}'))
        proxy=obj('proxy',f'{{isa = PBXContainerItemProxy; containerPortal = {uid("project")}; proxyType = 1; remoteGlobalIDString = {uid("Broadcast:target")}; remoteInfo = Broadcast;}}')
        deps.append(obj('dependency',f'{{isa = PBXTargetDependency; target = {uid("Broadcast:target")}; targetProxy = {proxy};}}'))
    target=obj(name+':target',f'{{isa = PBXNativeTarget; buildConfigurationList = {cl}; buildPhases = {arr(phases)}; buildRules = (); dependencies = {arr(deps) if deps else "()"}; name = {name}; productName = {name}; productReference = {product}; productType = "com.apple.product-type.{"app-extension" if ext else "application"}";}}')
    targets.append(target)
pg=obj('products',f'{{isa = PBXGroup; children = {arr(products)}; name = Products; sourceTree = "<group>";}}')
group=obj('group',f'{{isa = PBXGroup; children = {arr(list(files.values())+[pg])}; sourceTree = "<group>";}}')
configs=[obj('project:'+c,f'{{isa = XCBuildConfiguration; buildSettings = {{}}; name = {c};}}') for c in ['Debug','Release']]
cl=obj('project:configs',f'{{isa = XCConfigurationList; buildConfigurations = {arr(configs)}; defaultConfigurationIsVisible = 0; defaultConfigurationName = Release;}}')
pid=obj('project',f'{{isa = PBXProject; attributes = {{LastUpgradeCheck = 2600;}}; buildConfigurationList = {cl}; compatibilityVersion = "Xcode 14.0"; developmentRegion = en; hasScannedForEncodings = 0; knownRegions = (en,Base,); mainGroup = {group}; productRefGroup = {pg}; projectDirPath = ""; projectRoot = ""; targets = {arr(targets)};}}')
(project/'project.pbxproj').write_text('// !$*UTF8*$!\n{archiveVersion = 1; classes = {}; objectVersion = 56; objects = {\n'+'\n'.join(f'{k} = {v};' for k,v in objects.items())+'\n}; rootObject = '+pid+'; }\n')
schemes=project/'xcshareddata/xcschemes';schemes.mkdir(parents=True,exist_ok=True)
(schemes/'GameSeagullNext.xcscheme').write_text(f'''<?xml version="1.0" encoding="UTF-8"?>
<Scheme LastUpgradeVersion="2600" version="1.3"><BuildAction parallelizeBuildables="YES" buildImplicitDependencies="YES"><BuildActionEntries><BuildActionEntry buildForTesting="YES" buildForRunning="YES" buildForProfiling="YES" buildForArchiving="YES" buildForAnalyzing="YES"><BuildableReference BuildableIdentifier="primary" BlueprintIdentifier="{uid('GameSeagullNext:target')}" BuildableName="GameSeagullNext.app" BlueprintName="GameSeagullNext" ReferencedContainer="container:GameSeagullNext.xcodeproj"/></BuildActionEntry></BuildActionEntries></BuildAction><LaunchAction buildConfiguration="Debug" selectedDebuggerIdentifier="Xcode.DebuggerFoundation.Debugger.LLDB" selectedLauncherIdentifier="Xcode.IDEFoundation.Launcher.LLDB" launchStyle="0" useCustomWorkingDirectory="NO" ignoresPersistentStateOnLaunch="NO" debugDocumentVersioning="YES" debugServiceExtension="internal" allowLocationSimulation="YES"><BuildableProductRunnable runnableDebuggingMode="0"><BuildableReference BuildableIdentifier="primary" BlueprintIdentifier="{uid('GameSeagullNext:target')}" BuildableName="GameSeagullNext.app" BlueprintName="GameSeagullNext" ReferencedContainer="container:GameSeagullNext.xcodeproj"/></BuildableProductRunnable></LaunchAction><ProfileAction buildConfiguration="Release"/><AnalyzeAction buildConfiguration="Debug"/><ArchiveAction buildConfiguration="Release" revealArchiveInOrganizer="YES"/></Scheme>''')
for ext in [False,True]:
    info={'CFBundleDevelopmentRegion':'en','CFBundleExecutable':'$(EXECUTABLE_NAME)','CFBundleIdentifier':'$(PRODUCT_BUNDLE_IDENTIFIER)','CFBundleInfoDictionaryVersion':'6.0','CFBundleName':'$(PRODUCT_NAME)','CFBundleDisplayName':'Seagull Next Capture' if ext else 'Seagull Next','CFBundlePackageType':'XPC!' if ext else 'APPL','CFBundleShortVersionString':'$(MARKETING_VERSION)','CFBundleVersion':'$(CURRENT_PROJECT_VERSION)'}
    if ext: info['NSExtension']={'NSExtensionPointIdentifier':'com.apple.broadcast-services-upload','NSExtensionPrincipalClass':'$(PRODUCT_MODULE_NAME).SampleHandler','RPBroadcastProcessMode':'RPBroadcastProcessModeSampleBuffer'}
    else:info.update(LSRequiresIPhoneOS=True,UILaunchScreen={},UIBackgroundModes=['audio'],UISupportedInterfaceOrientations=['UIInterfaceOrientationPortrait','UIInterfaceOrientationLandscapeLeft','UIInterfaceOrientationLandscapeRight'],ITSAppUsesNonExemptEncryption=False)
    with (root/'ios'/('Broadcast' if ext else 'App')/'Info.plist').open('wb') as f:plistlib.dump(info,f)
privacy={'NSPrivacyTracking':False,'NSPrivacyTrackingDomains':[],'NSPrivacyCollectedDataTypes':[],'NSPrivacyAccessedAPITypes':[{'NSPrivacyAccessedAPIType':'NSPrivacyAccessedAPICategorySystemBootTime','NSPrivacyAccessedAPITypeReasons':['35F9.1']}]}
with (root/'ios/Shared/PrivacyInfo.xcprivacy').open('wb') as f:plistlib.dump(privacy,f)
print(project)
