# Final report status

The competition's 3.3.5.3 report is not yet present as a finished document.
Use only measured results for this submitted version.

Required sections to finish:

1. Problem, use case and originality; identify externally sourced IP/model.
2. Block diagram and PS/PL partition, clocks, resets, AXI/VDMA, DDR and ROI
   register contract.
3. Implementation of camera-to-HDMI, Lanczos/bilinear/edge ROI, pause/mouse
   review, PicoDet preprocessing and two-axis servo control.
4. Verification matrix with observable pass criteria, Golden comparisons,
   final-board screenshots/logs, and failure/rollback history.
5. Measured baseline versus FPGA-preprocessing latency and detection FPS under
   identical scene, model, threshold, thread count and sample size. Keep HDMI
   refresh separate from inference update rate.
6. Resource and routed timing results with the tool version (Vivado 2022.2)
   and why that version was used despite the guide's newer recommendation.
7. Reproduction from clean checkout, build dependencies and SD boot procedure.
8. AI collaboration record: prompts, responses, mistakes, corrections and the
   derivation/testing of each skill.

`ai_collaboration_record.md` is a curated, evidence-linked draft for item 8.
It is not a complete raw chat export; attach the original conversation excerpts
and missing board media before treating it as a final submission record.

Also still needed before final submission: a clean Vivado rebuild from this
folder, release-specific board log/video, license review, and a repository URL
or other submission mechanism named by the organizer. The one-page English
poster is conditional on reaching the final round.
