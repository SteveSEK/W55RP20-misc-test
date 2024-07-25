# [ W55RP20 Test ]


**W55RP20에 대한 다양한 테스트를 위한 프로젝트
Shell 기반의 명령어를 통해서 W55RP20의 기본 기능을 살펴본다.
![image](https://github.com/user-attachments/assets/75149c50-d844-4fe7-9945-0d2aea5dee94)  


** W5500 초기화
아래의 명령어를 사용하면, RP2040의 PIO 모드로 W5500 초기화를 진행한다.
![image](https://github.com/user-attachments/assets/b77f2945-bef2-4b7f-b11a-9d1990696bf8)  

'''
/bin/w5500 init spinormal 8 (N/A)
/bin/w5500 init spipio 32
/bin/w5500 init spipio 4
'''

** W5500 SPI 기본 통신
아래의 명령어를 사용하면, W5500 레지스터를 읽고 쓰는 작업이나 기본 정보를 확인 할 수 있다.
아래는 W5500의 MAC Address 레지스터에 Mac 주소를 직접 쓰는 예제이다.
![image](https://github.com/user-attachments/assets/aa38ec7d-a4c3-4d27-a6ec-695fb3d9781b)  


** lwIP와 연동
아래의 명령어를 사용하면, MAC Raw 모드를 이용하여 lwIP와 연동할 수 있다.
![image](https://github.com/user-attachments/assets/211c131e-0b5b-4400-a813-6865cf2f92af)  


** W5500 레지스터 확인
아래의 명령어를 사용하면, W5500의 Common 레지스터와 Socket 레지스터의 값들을 확인할 수 있다.
약, 100ms 단위로 업데이트를 하고 있고, LAN Cable Plug/Unplug 등의 간단한 동작을 통해 Ethernet PHY 링크의 변화 등을 파악할 수 있다.
![image](https://github.com/user-attachments/assets/0010801d-8f43-4fbd-9225-08bec120a335)
![image](https://github.com/user-attachments/assets/2fd3a4ca-406d-4f98-bebb-d9ab0cffa3b7)  

** RP2040 Memory Map 보기
아래의 명령어를 사용하면, RP2040의 Memory Map을 확인할 수 있다. 
![image](https://github.com/user-attachments/assets/7990fbfa-70ea-4818-aeb9-7ccb7ea2c060)  
'''
/bin/pico memory
/bin/pico memory 0 00000000 00000100
/bin/pico memory 1 40000000 40010000
/bin/pico memory 2 40000000 40010000
'''

** RP2040 GPIO 보기
아래의 명령어를 사용하면, RP2040의 GPIO 상태를 확인하거나 간단한 테스트를 할 수 있다. 
![image](https://github.com/user-attachments/assets/f2ace8cf-c1ad-453b-8af2-d1fccb14f028)

