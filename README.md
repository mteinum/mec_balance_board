# Balance Board Stability Monitor (M5 AtomS3)

A simple real-time stability indicator for shooters using the M5 AtomS3 and its built-in MPU6886 IMU.  
Designed for use on the [**MEC Balance Board**](https://shop.mec-shot.com/mec-balance-board) or similar wobble boards to help pistol and rifle shooters
train balance and stance stability.

The device measures board tilt (X/Y) at ~50 Hz, smooths the data, and displays a **traffic-light
feedback system** on the AtomS3’s built-in IPS screen:

- 🟩 **Green** — Very stable  
- 🟨 **Yellow** — Some sway  
- 🟥 **Red** — Too much movement  

This provides immediate and intuitive feedback during dry-fire or live-fire training.

## Features
- Uses the AtomS3's integrated **MPU6886 IMU**
- Calculates combined X/Y tilt in degrees relative to gravity
- Real-time exponential smoothing for clean, stable readings
- Full-screen color feedback
- Numeric tilt readout (°)
- **One-button calibration** — press the button to set your current stance as the zero reference
- Easy to tune thresholds for different shooters and disciplines

## Hardware
- **M5Stack AtomS3**  
  https://shop.m5stack.com/products/atoms3  
- **MEC Balance Board** (or any standard wobble/balance board)

## Use Case
Mount the AtomS3 firmly to the balance board (tape or 3D-printed holder).  
Stand on the board in your shooting stance.  
**Press the button** to calibrate — this sets your current position as the center/zero reference.  
Try to keep the screen **green** for as long as possible.

Great for:
- ISSF pistol stability training  
- Rifle hold/balance drills  
- Dry-fire sessions  
- Core balance training with visual feedback

## Code
The repository includes a full Arduino sketch:
- IMU reading and filtering  
- Tilt angle calculation  
- Color feedback rendering  
- Button-activated calibration system
- Optional tuning parameters

## License
MIT License

---

Contributions, improvements, and feature suggestions are welcome!
