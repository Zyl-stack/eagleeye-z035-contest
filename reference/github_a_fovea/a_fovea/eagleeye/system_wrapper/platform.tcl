# Usage: To re-create this platform project, launch xsct and run:
# source D:/EagleEye_FPGA_Competition/EagleEye/system_wrapper/platform.tcl

platform create -name {system_wrapper} \
    -hw {D:/EagleEye_FPGA_Competition/EagleEye/system_wrapper.xsa} \
    -proc {ps7_cortexa9_0} \
    -os {standalone} \
    -out {D:/EagleEye_FPGA_Competition/EagleEye}

platform write
platform active {system_wrapper}
platform config -updatehw {D:/EagleEye_FPGA_Competition/EagleEye/system_wrapper.xsa}

bsp reload
bsp setlib -name xilffs -ver 4.8
bsp config use_strfunc "2"
bsp write
catch {bsp regenerate}

platform generate
platform active {system_wrapper}
platform config -updatehw {D:/EagleEye_FPGA_Competition/EagleEye/system_wrapper.xsa}
platform generate -domains 
platform active {system_wrapper}
platform config -updatehw {D:/EagleEye_FPGA_Competition/EagleEye/system_wrapper.xsa}
platform generate
platform active {system_wrapper}
platform config -updatehw {D:/EagleEye_FPGA_Competition/EagleEye/system_wrapper.xsa}
platform generate -domains 
platform config -updatehw {D:/EagleEye_FPGA_Competition/EagleEye/system_wrapper.xsa}
platform generate -domains 
platform config -updatehw {D:/EagleEye_FPGA_Competition/EagleEye/system_wrapper.xsa}
platform generate -domains 
platform active {system_wrapper}
platform config -updatehw {D:/EagleEye_FPGA_Competition/EagleEye/system_wrapper.xsa}
platform generate -domains 
platform active {system_wrapper}
platform config -updatehw {D:/EagleEye_FPGA_Competition/EagleEye/system_wrapper.xsa}
platform generate -domains 
platform active {system_wrapper}
platform config -updatehw {D:/EagleEye_FPGA_Competition/EagleEye/system_wrapper.xsa}
platform generate -domains 
platform config -updatehw {D:/EagleEye_FPGA_Competition/EagleEye/system_wrapper.xsa}
platform generate -domains 
platform active {system_wrapper}
platform config -updatehw {D:/EagleEye_FPGA_Competition/EagleEye/system_wrapper.xsa}
platform generate -domains 
platform active {system_wrapper}
platform config -updatehw {D:/EagleEye_FPGA_Competition/EagleEye/system_wrapper.xsa}
platform generate -domains 
platform config -updatehw {D:/EagleEye_FPGA_Competition/EagleEye/system_wrapper.xsa}
platform generate -domains 
platform config -updatehw {D:/EagleEye_FPGA_Competition/EagleEye/system_wrapper.xsa}
platform generate -domains 
