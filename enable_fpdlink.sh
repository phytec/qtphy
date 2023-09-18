
#!/bin/bash

CSI_1_I2C_BUS=2
CSI_2_I2C_BUS=1
FPDLINK_ADDR=0x3d

#Entladen etwaiger Treiber
# echo INITALISIERUNG VON phyCAM-L
# rmmod onsemi_ar052x
# rmmod onsemi_core
# rmmod onsemi_regmap

#Initialisierung des Deserializers
# i2cset -y -f $CSI_1_I2C_BUS 0x3d 0x0c 0x01   #RX_PORT_CTL fuer Port0
i2cset -y -f $CSI_1_I2C_BUS 0x3d 0x0c 0x02   #RX_PORT_CTL fuer Port1

# i2cset -y -f $CSI_1_I2C_BUS 0x3d 0x4c 0x01    #I2C Schreibfreigabe PORT0
i2cset -y -f $CSI_1_I2C_BUS 0x3d 0x4c 0x12    #I2C Schreibfreigabe PORT1

i2cset -y -f $CSI_1_I2C_BUS 0x3d 0x58 0x5e    #PASSTHROUGH freigeben
i2cset -y -f $CSI_1_I2C_BUS 0x3d 0x5c 0x60    #In Register 0x5b muss die I2C-Adresse des Serializers auftauchen


#Konfigurieren des Serializers
i2cset -y -f $CSI_1_I2C_BUS 0x30 0x0e 0x87    #GPIO3 auf OUTPUT (nReset für Sensor)
i2cset -y -f $CSI_1_I2C_BUS 0x30 0x0d 0xf0    #GPIO3 auf LOW (nReset für Sensor)
i2cset -y -f $CSI_1_I2C_BUS 0x30 0x39 0x6c    #ID fuer Sensor setzen
i2cset -y -f $CSI_1_I2C_BUS 0x30 0x41 0x6c   #ID fuer Sensor setzen
i2cset -y -f $CSI_1_I2C_BUS 0x3d 0x5d 0x6c   #ID fuer Sensor setzen
i2cset -y -f $CSI_1_I2C_BUS 0x3d 0x65 0x6c   #ID fuer Sensor setzen
i2cset -y -f $CSI_1_I2C_BUS 0x30 0x3A 0xAC   #ID fuer EEPROM setzen
i2cset -y -f $CSI_1_I2C_BUS 0x30 0x42 0xAC   #ID fuer EEPROM setzen
i2cset -y -f $CSI_1_I2C_BUS 0x3d 0x5e 0xAC   #ID fuer EEPROM setzen
i2cset -y -f $CSI_1_I2C_BUS 0x3d 0x66 0xAC   #ID fuer EEPROM setzen

#MCLK auf 27,027 MHz @ 25 MHz REFCLK
#i2cset -y -f $CSI_1_I2C_BUS 0x30 0x06 0x43   #HS_CLK_DIV=4, M=3
#i2cset -y -f $CSI_1_I2C_BUS 0x30 0x07 0x6f   #N=111

#MCLK auf 26,916 MHz @ 24 MHz REFCLK
i2cset -y -f $CSI_1_I2C_BUS 0x30 0x06 0x43   #HS_CLK_DIV=4, M=3
i2cset -y -f $CSI_1_I2C_BUS 0x30 0x07 0x6B   #N=107

sleep 1
#Reset freigeben
i2cset -y -f $CSI_1_I2C_BUS 0x30 0x0d 0x78  #GPIO3 auf HIGH (nReset für Sensor)

#MIPI CSI-2 auf 800 Mbps @ 25 MHz REFCLK -> @24 MHz bei 768 MHz
i2cset -y -f $CSI_1_I2C_BUS 0x3d 0x1F 0x02   #800 Mbps / MIPI-CLK bei 400 MHz OK!
i2cset -y -f $CSI_1_I2C_BUS 0x3d 0x33 0x01   #4 Lanes, enalbe CSI output

#I2C Taktrate am Serializer auf ca. 400 kHz @ 25 MHz REFCLK
i2cset -y -f $CSI_1_I2C_BUS 0x30 0x0b 0x17
i2cset -y -f $CSI_1_I2C_BUS 0x30 0x0c 0x17

i2cset -y -f $CSI_1_I2C_BUS 0x3d 0x20 0x20   #Port forwarding

#Laden der Kameratreiber
# modprobe onsemi_regmap
# modprobe onsemi_core
# modprobe onsemi_ar052x
