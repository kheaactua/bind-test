# Overview

Learning how to bind to interfaces

# Build

## Manual

Create your build configuration in a `CMakeUserPreset.json` file, use it as follows (using the `eno1` config)

```bash
cmake -S. --preset eno1 \
  && cmake --build --preset default
  && build/default/bind-test
```

## AOSP

Add the following to a local manifest:
```xml
<remote name="matt" fetch="https://github.com/kheaactua" />
<project name="bind-test" path="external/bind-test" remote="matt" revision="refs/heads/main" />
```

Build with:
```
mm bind-test.{vendor,system}
```

# Config

For local configs, create a `CMakeUserPresets.json` file.  Example,
```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "clang",
      "inherits": "linux",
      "hidden": true,
      "environment": {
        "CC": "clang",
        "CXX": "clang++",
        "WORKSPACE": "/tmp/qnx-workspace"
      },
      "toolchainFile": "$env{WORKSPACE}/cmake/host-toolchainfile.cmake",
      "cacheVariables": {
        "CMAKE_CXX_CLANG_TIDY": {
          "type": "STRING",
          "value": "clang-tidy;-header-filter='${sourceDir}/*';-checks='modernize-*,readability-delete-null-pointer,readability-duplicate-include,readability-convert-member-functions-to-static,readability-implicit-bool-conversion,readability-make-member-function-const,readability-misleading-indentation,readability-non-const-parameter,readability-qualified-auto,readability-redundant-control-flow,readability-simplify-boolean-expr,readability-redundant-preprocessor,readability-string-compare,readability-static-accessed-through-instance,readability-const-return-type,readability-container-contains,readability-container-data-pointer,readability-container-size-empty,readability-avoid-const-params-in-decls,performance-unnecessary-copy-initialization,performance-unnecessary-value-param,performance-move-const-arg,performance-for-range-copy,performance-trivially-destructible,performance-inefficient-string-concatenation,performance-inefficient-vector-operation,performance-implicit-conversion-in-loop,performance-faster-string-find,misc-new-delete-overloads,misc-misplaced-const,misc-misleading-identifier,misc-throw-by-value-catch-by-reference'"
        },
        "CMAKE_PREFIX_PATH": {
          "type": "STRING",
          "value": "<QNX Boost Stage>"
        }
      }
    },
    {
      "name": "eno1",
      "inherits": "clang",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/eno1",
      "displayName": "eno1",
      "description": "Build for eno1",
      "cacheVariables": {
        "INTERFACE_IP": {
          "type": "STRING",
          "value": "<LAN IP>"
        },
        "INTERFACE_NAME": {
          "type": "STRING",
          "value": "eno1"
        }
      }
    },
    {
      "name": "qc",
      "inherits": "clang",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/qc",
      "displayName": "QC Network",
      "description": "Build for enp0s31f6",
      "cacheVariables": {
        "INTERFACE_IP": {
          "type": "STRING",
          "value": "10.1.0.100"
        },
        "INTERFACE_NAME": {
          "type": "STRING",
          "value": "enp0s31f6"
        },
        "CMAKE_PREFIX_PATH": {
          "type": "STRING",
          "value": "<QNX Boost Stage>"
        }
      }
    },
    {
      "name": "qnx-local",
      "inherits": "default",
      "hidden": true,
      "displayName": "QNX Config",
      "architecture": {
        "value": "aarch64le",
        "strategy": "external"
      },
      "environment": {
        "OUT_DIR": "$env{WORKSPACE}/output"
      },
      "toolchainFile": "$env{WORKSPACE}/cmake/hlos-toolchainfile.cmake",
      "cacheVariables": {
        "Boost_USE_STATIC_RUNTIME": {
          "type": "BOOL",
          "value": "On"
        },
        "Boost_DEBUG": {
          "type": "BOOL",
          "value": "On"
        },
        "CMAKE_PREFIX_PATH": {
          "type": "STRING",
          "value": "<QNX Boost stage>"
        }
      }
    },
    {
      "name": "emac0-local",
      "inherits": "qnx-local",
      "displayName": "Build for emac0",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/emac0",
      "installDir": "${sourceDir}/stage/emac0",
      "cacheVariables": {
        "INTERFACE_IP": {
          "type": "STRING",
          "value": "10.1.0.3"
        },
        "INTERFACE_NAME": {
          "type": "STRING",
          "value": "emac0"
        }
      }
    },
    {
      "name": "vp1-local",
      "inherits": "qnx-local",
      "displayName": "Build for VP1",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/vp1",
      "installDir": "${sourceDir}/stage/vp1",
      "cacheVariables": {
        "INTERFACE_IP": {
          "type": "STRING",
          "value": "10.7.0.3"
        },
        "INTERFACE_NAME": {
          "type": "STRING",
          "value": "vp1"
        }
      }
    },
    {
      "name": "vp2-local",
      "inherits": "qnx-local",
      "displayName": "Non build for vp2",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/vp2",
      "installDir": "${sourceDir}/stage/vp2",
      "cacheVariables": {
        "INTERFACE_IP": {
          "type": "STRING",
          "value": "10.6.0.3"
        },
        "INTERFACE_NAME": {
          "type": "STRING",
          "value": "vp2"
        }
      }
    }
  ],
  "buildPresets": [
    {
      "name": "eno1",
      "configurePreset": "eno1"
    },
    {
      "name": "qc",
      "configurePreset": "qc"
    },
    {
      "name": "emac0-local",
      "configurePreset": "emac0-local"
    },
    {
      "name": "vp1-local",
      "configurePreset": "vp1-local"
    },
    {
      "name": "vp2-local",
      "configurePreset": "vp2-local"
    }
  ]
}
```

Modify the paths for your local system, and then you can build for the QNX with:
```bash
export QNX_HOST=<QNX HOST PATH> QNX_TARGET=<QNX_TARGET PATH>
if_name=emac0;
cmake -S . --preset=${if_name}-local \
	&& cmake --build --preset ${if_name}-local \
	&& cmake --install build/${if_name} \
	&& scp /home/matt/workspace/bind-test/stage/${if_name}/aarch64le/bin/bind-test qnx:/bin/
```

Or locally,
```bash
if_name=eno1;
cmake --build --preset ${if_name} && build/${if_name}/bind-test
```

[modeline]: # ( vim: set fenc=utf-8 spell spl=en ts=2 sw=2 noexpandtab sts=0 ff=unix : )
