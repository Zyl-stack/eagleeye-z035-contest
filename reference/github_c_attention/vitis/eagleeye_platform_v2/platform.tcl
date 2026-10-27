# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct C:\Users\LENOVO\workspace_vitis3\eagleeye_platform_v2\platform.tcl
# 
# OR launch xsct and run below command.
# source C:\Users\LENOVO\workspace_vitis3\eagleeye_platform_v2\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {eagleeye_platform_v2}\
-hw {C:\Users\LENOVO\Documents\EagleEye_Vitis_Workspace\system_wrapper.xsa}\
-proc {ps7_cortexa9_0} -os {standalone} -out {C:/Users/LENOVO/workspace_vitis3}

platform write
platform generate -domains 
platform active {eagleeye_platform_v2}
platform generate
platform active {eagleeye_platform_v2}
platform active {eagleeye_platform_v2}
catch {platform remove eagleeye_platform}
platform generate
