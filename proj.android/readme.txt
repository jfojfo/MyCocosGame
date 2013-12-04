How to build in eclipse (with cocos2d-x-2.2):
1.first import existing android project(proj.android)
2.goto project property, choose C/C++ Build, 
  change Build command to:
  E:\work\android-ndk-r9\ndk-build.cmd NDK_MODULE_PATH="E:\work\cocos2d-x-2.2;E:\work\cocos2d-x-2.2\cocos2dx\platform\third_party\android\prebuilt"
3.cp ../Resources/* assets/
4.Build

Modification to jni/Application.mk
1.add '-std=c++11' flag

Modification to jni/Android.mk
1.add all .cpp files under ../Classes/
MY_CPP_LIST := hellocpp/main.cpp
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/../../Classes/*.cpp)
LOCAL_SRC_FILES := $(MY_CPP_LIST:$(LOCAL_PATH)/%=%)

