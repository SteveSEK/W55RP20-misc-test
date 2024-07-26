# [ W55RP20 Test ]

**Project for Various Tests on W55RP20**

## Quick Start Firmware
* You can start this quickly with these compiled files.
  - **[Firmware for W55RP20 (452KB)](W55RP20.uf2)**
  - **[Firmware for W5500-EVB-PICO (452KB)](W5500-EVB-PICO.uf2)**


Explore the basic functions of the W55RP20 through shell-based commands.  
![image](https://github.com/user-attachments/assets/75149c50-d844-4fe7-9945-0d2aea5dee94)  

### **W5500 Initialization**
Use the following commands to initialize the W5500 in PIO mode on the RP2040.
![image](https://github.com/user-attachments/assets/b77f2945-bef2-4b7f-b11a-9d1990696bf8)  

```sh
/bin/w5500 init spipio 32
/bin/w5500 init spipio 4
/bin/w5500 init spinormal 8 (N/A)
```

### **Basic SPI Communication with W5500**
Use the following commands to read and write to the W5500 registers or check basic information. Below is an example of writing a MAC address directly to the MAC Address register of the W5500.
![image](https://github.com/user-attachments/assets/aa38ec7d-a4c3-4d27-a6ec-695fb3d9781b)  

### **Integration with lwIP**
Use the following commands to integrate with lwIP using MAC Raw mode.
![image](https://github.com/user-attachments/assets/211c131e-0b5b-4400-a813-6865cf2f92af)  

### **Checking W5500 Registers**
Use the following commands to check the values of the W5500's Common and Socket registers. The updates occur approximately every 100ms, and you can observe changes in the Ethernet PHY link status through simple actions like plugging/unplugging the LAN cable.
![image](https://github.com/user-attachments/assets/0010801d-8f43-4fbd-9225-08bec120a335)
![image](https://github.com/user-attachments/assets/2fd3a4ca-406d-4f98-bebb-d9ab0cffa3b7)  

### **Checking RP2040 Memory Map**
Use the following commands to check the memory map of the RP2040.
![image](https://github.com/user-attachments/assets/7990fbfa-70ea-4818-aeb9-7ccb7ea2c060)  
```sh
/bin/pico memory
/bin/pico memory 0 00000000 00000100
/bin/pico memory 1 40000000 40010000
/bin/pico memory 2 40000000 40010000
```

### **Checking RP2040 GPIO**
Use the following commands to check the status of the RP2040's GPIO or perform simple tests.
![image](https://github.com/user-attachments/assets/f2ace8cf-c1ad-453b-8af2-d1fccb14f028)


### **Useful CLI Commands**
```sh
/bin/reboot

/bin/w5500 init spipio 32
/bin/w5500 init spipio 4
/bin/w5500 init spinormal 8 (X)

/bin/w5500 info
/bin/w5500 readbuff 0009 16
/bin/w5500 writebuff 0009 0008dcaabbee

/bin/w5500 register

/bin/w5500 lwip 

/bin/pico memory
/bin/pico memory 0 0 0 
/bin/pico memory 0 00000000 00000100
/bin/pico memory 1 40000000 40010000
/bin/pico memory 2 40000000 40010000

/bin/pico pins
/bin/pico gpiotest
```

