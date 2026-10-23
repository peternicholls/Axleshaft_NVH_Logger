# Marked-shaft NVH experiment: hardware register

Updated 2026-10-23. User reports these parts have been purchased and are expected on **2026-10-24**. Delivery, quantities, specifications and fit have not yet been verified. This register concerns the GY-521/optical timing experiment, not a qualified vehicle logger or wheel-balancer design.

| Purchased item | Intended role | Status / details still needed |
|---|---|---|
| Motor mount | Secure the drive motor | Ordered; motor interface and fixing dimensions unknown |
| Pulley wheel | Part of shaft/drive arrangement; possible mark carrier if suitable | Ordered; bore, diameter, attachment and drive arrangement unconfirmed |
| Bearings | Support rotating steel rod | Ordered; bore/type, quantity and fit unknown |
| Bearing mounts | Fix bearing supports to the rig | Ordered; dimensions/alignment provisions unknown |
| Steel rod | Rotating test shaft | Ordered; diameter, length and end details unknown |
| Washers and screws / end-retention arrangement | User-described retention at the rod ends/bearings | Whether included and the exact assembly/retention method need confirmation against the supplied parts |

The user describes rod ends retained using washers and screws at the bearings. This records the intended arrangement, not an engineering verification of how those parts should be clamped. Inspect the actual components/supplier assembly information before finalising the stack-up.

## Once the parts arrive

1. Record photos, part identifiers and measured rod/bearing/pulley interfaces; confirm what was actually supplied.
2. Establish the motor-to-shaft drive arrangement and whether belt/coupling, base fixings and motor/supply/speed control are already available. Their purchase/availability is not established by this update.
3. Document assembly, axial retention, alignment and hand-rotation behaviour before powered trials. Do not infer a permitted speed or contact load from the purchase list.
4. Choose the single white-mark location and a fixed IR mount/gap. Check a clear view throughout rotation and record pot/polarity settings.
5. Design the once-per-revolution cam/tap and stationary IMU bracket, with adjustable mark-to-tap phase. These features are not yet established by the purchased parts.
6. Feed dimensions and setup photos into [NVH-TIME-01](experiments/timing-protocol.md) and the [next-phase plan](next-phase-plan.md). Keep the existing GY-521 bring-up and timing-instrumentation work separate from mechanical acceptance.

No firmware or test procedure change is needed merely because hardware has been ordered. Exact operating speeds, geometry, mounting and timing-reference capability remain to be established from the assembled rig and recorded evidence.
