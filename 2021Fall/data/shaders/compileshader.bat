@echo off

set str=%1
set str=%str:.=%.spv

C:\VulkanSDK\1.2.189.2\Bin\glslc.exe %1 -o %str%
