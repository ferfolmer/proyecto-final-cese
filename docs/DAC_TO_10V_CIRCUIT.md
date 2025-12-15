# DAC to 0-10V Amplifier Circuit

## Overview
This document describes the circuit to convert the STM32F446RE DAC output (0-3.3V) to the industry-standard 0-10V control signal for horticultural LED drivers.

---

## Circuit Design

### Non-Inverting OpAmp Configuration

```
         R1 (20kΩ)
         ┌────────┐
         │        │
DAC ─────┤+       │
(0-3.3V) │   OP   ├──┬───── 0-10V Output
      ┌──┤-       │  │
      │  │        │  │
      │  └────────┘  │
      │              │
      │    R2 (10kΩ) │
      └──────────────┤
                     │
                    GND

Gain = 1 + (R1/R2) = 1 + (20k/10k) = 3.0

Output = DAC_voltage × 3.0
```

### Component Selection

**OpAmp**: Rail-to-rail output, single supply
- Option 1: **TL071** (low-cost, standard)
- Option 2: **LM358** (dual OpAmp, 2 channels per IC)
- Option 3: **MCP6002** (rail-to-rail, better for low voltage)

**Power Supply**: +12V or +15V (must be >10V for proper headroom)

**Resistors**:
- R1 = 20kΩ (1% tolerance recommended)
- R2 = 10kΩ (1% tolerance recommended)

---

## Voltage Mapping

```
DAC Output    →    Amplifier Output    →    LED Driver
---------------------------------------------------------
   0V         →         0V              →       0% intensity
   1.65V      →         5V              →      50% intensity
   3.3V       →         9.9V ≈ 10V      →     100% intensity
```

**Note**: Actual gain slightly less than 3.0 due to resistor tolerances and OpAmp limitations. Calibration recommended.

---

## Circuit for 2 Channels (DAC1 + DAC2)

```
STM32F446RE                        LED Drivers
┌─────────────┐
│             │
│ DAC1 (PA4) ─┼──→ OpAmp 1 ──→ 0-10V ──→ LED Driver Ch0 (Red)
│             │
│ DAC2 (PA5) ─┼──→ OpAmp 2 ──→ 0-10V ──→ LED Driver Ch1 (Blue)
│             │
│      GND ───┼──→ Common ground
│             │
└─────────────┘
```

---

## PCB Layout Considerations

1. **Keep OpAmp close to DAC output** - minimize noise pickup
2. **Star ground** - connect all grounds to single point
3. **Decoupling capacitors**:
   - 100nF ceramic near OpAmp power pins
   - 10µF electrolytic on OpAmp supply rail
4. **Output filtering** (optional):
   - Add 100nF capacitor between output and GND for noise reduction

---

## Calibration Procedure

The amplifier gain may not be exactly 3.0 due to component tolerances:

### Step 1: Measure actual gain
```c
// Set DAC to known voltage
OutputDriver_SetChannel(output, CHANNEL_0, 50.0f);  // Should be 1.65V DAC

// Measure actual output voltage with multimeter
// If measured = 4.95V instead of 5.0V:
//   Actual gain = 4.95V / 1.65V = 3.00
```

### Step 2: Software compensation (optional)
If gain error is significant, compensate in software:
```c
// Apply correction factor in OutputDriver
float correctedPercent = requestedPercent * (3.0 / actualGain);
```

---

## Alternative: Use DAC Module

Instead of building the circuit, you can use a commercial module:
- **Search**: "0-10V DAC module" or "PLC analog output module"
- **Advantage**: Pre-calibrated, isolated, industrial-grade
- **Disadvantage**: Higher cost (~$10-20 vs <$2 DIY)

---

## Testing Without LED Drivers

For initial testing, you can verify the circuit with just a multimeter:

```c
// Test code
OutputDriver_Init(output);

OutputDriver_SetChannel(output, CHANNEL_0, 0.0f);
// Measure: Should read ~0V

OutputDriver_SetChannel(output, CHANNEL_0, 50.0f);
// Measure: Should read ~5V

OutputDriver_SetChannel(output, CHANNEL_0, 100.0f);
// Measure: Should read ~10V
```

---

## Future Expansion: PWM Channels

For channels 2-3 (if using PWM instead of DAC):

```
STM32F446RE Timer PWM
│
└─→ PWM signal (0-100% duty cycle) ─→ LED Driver PWM input
```

Many LED drivers accept both 0-10V **and** PWM inputs:
- PWM frequency: 1-10 kHz typical
- PWM voltage: 0-5V or 0-10V (check driver datasheet)

---

## Safety Notes

⚠️ **Do NOT connect DAC directly to LED driver!**
- DAC max output: 3.3V
- LED driver expects: 0-10V
- **Result**: Only 33% maximum brightness possible

⚠️ **Verify LED driver input impedance**
- Most 0-10V inputs: 10kΩ - 100kΩ (high impedance, OK)
- OpAmp can drive this easily
- If impedance <10kΩ, use a buffer OpAmp

---

## References

- IEC-60929 Annex E: 0-10V Dimming Standard
- STM32F446RE Datasheet: DAC specifications (section 6.3.24)
- AN4566: Extending DAC performance of STM32 microcontrollers

---

**Document Status**: Hardware design reference
**Last Updated**: 2025-11-03
