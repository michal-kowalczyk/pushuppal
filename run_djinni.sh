#! /usr/bin/env bash
deps/djinni/src/run-assume-built \
    --java-out android/PushUpPal/app/src/main/java/pl/ekk/mkk/pushuppal/generated \
    --java-package pl.ekk.mkk.pushuppal.generated \
    --ident-java-field mFooBar \
    \
    --jni-out "native/PushUpPal/glue-code/jni/generated" \
    --ident-jni-class NativeFooBar \
    --ident-jni-file NativeFooBar \
    \
    --cpp-out "native/PushUpPal/glue-code/interfaces/generated" \
    --cpp-namespace generated \
    --ident-cpp-enum-type foo_bar \
    \
    --objc-out "native/PushUpPal/glue-code/objc/generated" \
    --objcpp-out "native/PushUpPal/glue-code/objc/generated" \
    --objc-swift-bridging-header "PushUpPal-Glue-Code-Bridging-Header" \
    --objc-type-prefix PUP \
    \
    --idl PushUpPal.djinni
