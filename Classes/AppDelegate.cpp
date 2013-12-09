#include "AppDelegate.h"
#include "cocos2d.h"
#include "SimpleAudioEngine.h"
#include "ScriptingCore.h"
#include "jsb_cocos2dx_auto.hpp"
#include "jsb_cocos2dx_extension_auto.hpp"
#include "XMLHTTPRequest.h"
#include "JSB_AUTO.h"

USING_NS_CC;
using namespace CocosDenshion;

static bool firstTime = true;

extern "C" {
    const char *getCCBDirectoryPath();
#ifdef __ANDROID__
    const char * getCCBDirectoryPath() {
        std::string path = cocos2d::CCFileUtils::sharedFileUtils()->getWritablePath();
        path += "/ccb";
        return path.c_str();
    }
#endif
    
    static void setResolutionSizes(bool isTablet, bool isRetina) {
        CCConfiguration *conf = CCConfiguration::sharedConfiguration();
        
        bool isPortrait = conf->getBool("cocos2d.orientation.isPortrait", true);
        double tabletW = conf->getNumber("cocos2d.designSize.tabletWidth", 0);
        double tabletH = conf->getNumber("cocos2d.designSize.tabletHeight", 0);
        double phoneW = conf->getNumber("cocos2d.designSize.phoneWidth", 0);
        double phoneH = conf->getNumber("cocos2d.designSize.phoneHeight", 0);
        
        if(tabletH == 0 || tabletW == 0) {
            if(isPortrait) {
                tabletH = 1024; tabletW = 768;
            } else {
                tabletH = 768; tabletW = 1024;
            }
        }
        
        if(phoneH == 0 || phoneW == 0) {
            if(isPortrait) {
                phoneH = 480; phoneW = 320;
            } else {
                phoneH = 320; phoneW = 480;
            }
        }
        
        CCSize designSize, resourceSize;

        // Is tablet or not
        designSize = CCSizeMake((isTablet) ? tabletW : phoneW, (isTablet) ? tabletH : phoneH);
        
        // Is retina -> scale by 2
        CCDirector::sharedDirector()->setContentScaleFactor((isRetina) ? 2.0 : 1.0);
        
        // Is portrait -> kResolutionFixedWidth, !portait -> kResolutionFixedHeight
        CCEGLView::sharedOpenGLView()->setDesignResolutionSize(designSize.width,
                                                               designSize.height,
                                                               isPortrait ? kResolutionFixedWidth : kResolutionFixedHeight);
        
    }
}

void handle_signal(int signal)
{
    static int internal_state = 0;
    ScriptingCore* sc = ScriptingCore::getInstance();
    // should start everything back
    CCDirector* director = CCDirector::sharedDirector();
    if (director->getRunningScene()) {
        director->popToRootScene();
    } else {
        CCPoolManager::sharedPoolManager()->finalize();
        if (internal_state == 0) {
            //sc->dumpRoot(NULL, 0, NULL);
            sc->start();
            internal_state = 1;
        } else {
            sc->runScript("main.js");
            internal_state = 0;
        }
    }
}

static const char* getCCBWriteablePath() {
    std::string writeablePath = CCFileUtils::sharedFileUtils()->getWritablePath();
    writeablePath += "/ccb/";
    return writeablePath.c_str();
}

AppDelegate::AppDelegate(): isRetina(false), isIPhone(false), _resolution("")
{
}

AppDelegate::~AppDelegate()
{
    CCScriptEngineManager::sharedManager()->purgeSharedManager();
    //    SimpleAudioEngine::end();
}

bool AppDelegate::applicationDidFinishLaunching()
{
    // initialize director
    CCDirector *pDirector = CCDirector::sharedDirector();
    pDirector->setOpenGLView(CCEGLView::sharedOpenGLView());
    
    // turn on display FPS
    //pDirector->setDisplayStats(true);
    
    // set FPS. the default value is 1.0/60 if you don't call this
    pDirector->setAnimationInterval(1.0 / 60);
    
    this->initializePaths();
    this->initializeJavaScript();
    
    return true;
}

void AppDelegate::initializePaths()
{
    CCSize screenSize = CCEGLView::sharedOpenGLView()->getFrameSize();
    CCConfiguration *conf = CCConfiguration::sharedConfiguration();

	if(!conf->getBool("cocos2d.orientation.isPortrait", true)) {
		screenSize = CCSizeMake(screenSize.height, screenSize.width);
	}

	std::vector<std::string> resDirOrders;
	TargetPlatform platform = CCApplication::sharedApplication()->getTargetPlatform();


	if (platform == kTargetIphone || platform == kTargetIpad)
	{
		// CocosBuilderReader search path
		std::vector<std::string> searchPaths = CCFileUtils::sharedFileUtils()->getSearchPaths();
		searchPaths.insert(searchPaths.begin(), "Published files iOS");
		searchPaths.insert(searchPaths.begin(), getCCBDirectoryPath());
		CCFileUtils::sharedFileUtils()->setSearchPaths(searchPaths);

		if (screenSize.height > 1136)
		{
			_resolution = "iPad";
			setResolutionSizes(true, true);
			resDirOrders.push_back("resources-ipadhd");
			resDirOrders.push_back("resources-ipad");
			resDirOrders.push_back("resources-iphonehd");
			isIPhone = false;
			isRetina = true;
            
            // CCBReader::setResolutionScale(2) * sharedDirector()->setContentScaleFactor(1) = 2
            // == (iPad HD)
            // CocosBuilder scaling = 1,2,2,4 (iPhone, iPhoneRetina, iPad, iPadHD)
			cocos2d::extension::CCBReader::setResolutionScale(2);
            
		} else if(screenSize.height > 1024) {
			_resolution = "iPhone";
			setResolutionSizes(false, true);
			resDirOrders.push_back("resources-iphonehd");
			resDirOrders.push_back("resources-iphone");
			isIPhone = true;
			isRetina = true;
		}
		else if (screenSize.height > 960)
		{
			_resolution = "iPad";
			setResolutionSizes(true, false);
			resDirOrders.push_back("resources-ipad");
			resDirOrders.push_back("resources-iphonehd");
			isIPhone = false;
			isRetina = false;
            
            // CCBReader::setResolutionScale(2) * sharedDirector()->setContentScaleFactor(1) = 2
            // == (iPad)
            // CocosBuilder scaling = 1,2,2,4 (iPhone, iPhoneRetina, iPad, iPadHD)
			cocos2d::extension::CCBReader::setResolutionScale(2);

		}
		else if (screenSize.height > 480)
		{
			_resolution = "iPhone";
			setResolutionSizes(false, true);
			resDirOrders.push_back("resources-iphonehd");
			resDirOrders.push_back("resources-iphone");
			isIPhone = true;
			isRetina = true;
		}
		else
		{
			_resolution = "iPhone";
			setResolutionSizes(false, false);
			resDirOrders.push_back("resources-iphone");
			isIPhone = true;
			isRetina = false;
		}

	}
	else if (platform == kTargetAndroid || platform == kTargetWindows)
	{
		int dpi = -1;
		dpi = CCDevice::getDPI();

		if(dpi > 300) { // retina
			if (screenSize.height > 1920) {
				_resolution = "xlarge";
				setResolutionSizes(true, true);
				resDirOrders.push_back("resources-xlarge");
				resDirOrders.push_back("resources-large");
				resDirOrders.push_back("resources-medium");
				resDirOrders.push_back("resources-small");
			} else {
				_resolution = "large";
				setResolutionSizes(false, true);
				resDirOrders.push_back("resources-large");
				resDirOrders.push_back("resources-medium");
				resDirOrders.push_back("resources-small");
			}
		} else { // non retina
			if (screenSize.height > 960)
			{
				_resolution = "large";
				setResolutionSizes(true, false);
				resDirOrders.push_back("resources-large");
				resDirOrders.push_back("resources-medium");
				resDirOrders.push_back("resources-small");
                
                // CCBReader::setResolutionScale(2) * sharedDirector()->setContentScaleFactor(2) = 4
                // == (iPad retina)
				cocos2d::extension::CCBReader::setResolutionScale(2);
			}
			else if (screenSize.height > 768)
			{
				_resolution = "medium";
				setResolutionSizes(true, false);
				resDirOrders.push_back("resources-medium");
				resDirOrders.push_back("resources-small");
			}
			else if (screenSize.height > 480)
			{
				_resolution = "small";
				setResolutionSizes(false, false);
				resDirOrders.push_back("resources-small");
			}
			else
			{
				setResolutionSizes(false, false);
				_resolution = "xsmall";
				resDirOrders.push_back("resources-xsmall");
			}

		}
	}

	CCFileUtils *pFileUtils = CCFileUtils::sharedFileUtils();
	pFileUtils->setSearchResolutionsOrder(resDirOrders);

	std::vector<std::string> searchPaths = pFileUtils->getSearchPaths();
	searchPaths.insert(searchPaths.begin(), pFileUtils->getWritablePath());
	pFileUtils->setSearchPaths(searchPaths);
}

void AppDelegate::initializeJavaScript()
{
    ScriptingCore* sc = ScriptingCore::getInstance();
    sc->addRegisterCallback(register_all_cocos2dx);
    sc->addRegisterCallback(register_all_cocos2dx_extension);
    sc->addRegisterCallback(register_cocos2dx_js_extensions);
    sc->addRegisterCallback(register_all_cocos2dx_extension_manual);
    sc->addRegisterCallback(register_CCBuilderReader);
    sc->addRegisterCallback(jsb_register_chipmunk);
    sc->addRegisterCallback(jsb_register_system);
    sc->addRegisterCallback(JSB_register_opengl);
    sc->addRegisterCallback(MinXmlHttpRequest::_js_register);
    sc->addRegisterCallback(register_all);
    
    sc->start();
    
    CCScriptEngineProtocol *pEngine = ScriptingCore::getInstance();
    CCScriptEngineManager::sharedManager()->setScriptEngine(pEngine);
    ScriptingCore::getInstance()->runScript("main.js");
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground()
{
    CCDirector::sharedDirector()->stopAnimation();
    SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
    SimpleAudioEngine::sharedEngine()->pauseAllEffects();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground()
{
    CCDirector::sharedDirector()->startAnimation();
    SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();
    SimpleAudioEngine::sharedEngine()->resumeAllEffects();
}
