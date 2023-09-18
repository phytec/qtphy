#!/bin/bash

CSI_1_I2C_BUS=2
CSI_2_I2C_BUS=1

BOOTENV_FILE="/boot/bootenv.txt"

recommended_dtos=()

human_readable=true

VM016_ADDR=0x10
VM017_ADDR=0x36 
FPDLINK_ADDR=0x3d

CAMERA_NAMES=("VM016" "VM017" "VM020")
CAMERA_ADDRS=("0x10" "0x36" "0x36")
CAMERA_UIDS=("0x0356" "0x0457" "foo") # TBD VM020 UID
CAMERA_OVERLAYS=(
  "imx8mp-vm016"
  "imx8mp-vm017"
  "imx8mp-vm020"
)  

overlaysToRemove=(
    " imx8mp-isi-csi1.dtbo"
    " imx8mp-isi-csi2.dtbo"
    " imx8mp-isp-csi1.dtbo"
    " imx8mp-isp-csi2.dtbo"
    " imx8mp-vm016-csi1-fpdlink-port0.dtbo"
    " imx8mp-vm016-csi1-fpdlink-port1.dtbo"
    " imx8mp-vm016-csi1.dtbo"
    " imx8mp-vm016-csi2-fpdlink-port0.dtbo"
    " imx8mp-vm016-csi2-fpdlink-port1.dtbo"
    " imx8mp-vm016-csi2.dtbo"
    " imx8mp-vm017-csi1-fpdlink-port0.dtbo"
    " imx8mp-vm017-csi1-fpdlink-port1.dtbo"
    " imx8mp-vm017-csi1.dtbo"
    " imx8mp-vm017-csi2-fpdlink-port0.dtbo"
    " imx8mp-vm017-csi2-fpdlink-port1.dtbo"
    " imx8mp-vm017-csi2.dtbo"
    " imx8mp-vm020-csi1.dtbo"
    " imx8mp-vm020-csi2.dtbo"
)

enable_fpdlink() {
  local i2c_bus="$1"
  local port="$2"

  # Initialisierung des Deserializers
  if [ $port = 0 ]; then
    i2cset -y -f $i2c_bus $FPDLINK_ADDR 0x0c 0x01  #RX_PORT_CTL Port0
    i2cset -y -f $i2c_bus $FPDLINK_ADDR 0x4c 0x01  #I2C Schreibfreigabe PORT0
  elif [ $port = "1" ]; then
    i2cset -y -f $i2c_bus 0x3d 0x0c 0x02   #RX_PORT_CTL Port1
    i2cset -y -f $i2c_bus 0x3d 0x4c 0x12    #I2C Schreibfreigabe PORT1
  else
    echo "ERROR: invalid fpdlink PORT"
    exit -1
  fi
  i2cset -y -f $i2c_bus $FPDLINK_ADDR 0x58 0x5e  #PASSTHROUGH freigeben
  i2cset -y -f $i2c_bus $FPDLINK_ADDR 0x5c 0x60  #In Register 0x5b muss die I2C-Adresse des Serializers auftauchen

  serializer_addr=$(i2cget -y -f $i2c_bus $FPDLINK_ADDR 0x5b)

  # No serializer found on this port -> return
  if [[ $serializer_addr -gt 0 ]]; then
    sleep 0.2
    #Konfigurieren des Serializers
    i2cset -y -f $i2c_bus $serializer_addr 0x0e 0x87  #GPIO3 auf OUTPUT (nReset für Sensor)
    i2cset -y -f $i2c_bus $serializer_addr 0x0d 0xf0  #GPIO3 auf LOW (nReset für Sensor)
    i2cset -y -f $i2c_bus $serializer_addr 0x39 0x6c  #ID fuer Sensor setzen
    i2cset -y -f $i2c_bus $serializer_addr 0x41 0x6c  #ID fuer Sensor setzen
    i2cset -y -f $i2c_bus $FPDLINK_ADDR 0x5d 0x6c     #ID fuer Sensor setzen
    i2cset -y -f $i2c_bus $FPDLINK_ADDR 0x65 0x6c     #ID fuer Sensor setzen
    i2cset -y -f $i2c_bus $serializer_addr 0x42 0xAC  #ID fuer EEPROM setzen
    i2cset -y -f $i2c_bus $FPDLINK_ADDR 0x5e 0xAC     #ID fuer EEPROM setzen
    i2cset -y -f $i2c_bus $FPDLINK_ADDR 0x66 0xAC     #ID fuer EEPROM setzen
    #MCLK auf 27,027 MHz @ 25 MHz REFCLK
    #i2cset -y -f $i2c_bus $serializer_addr 0x06 0x43 #HS_CLK_DIV=4, M=3
    #i2cset -y -f $i2c_bus $serializer_addr 0x07 0x6f #N=111

    #MCLK auf 26,916 MHz @ 24 MHz REFCLK
    i2cset -y -f $i2c_bus $serializer_addr 0x06 0x43  #HS_CLK_DIV=4, M=3
    i2cset -y -f $i2c_bus $serializer_addr 0x07 0x6B  #N=107

    sleep 1
    #Reset freigeben
    i2cset -y -f $i2c_bus $serializer_addr 0x0d 0x78  #GPIO3 auf HIGH (nReset für Sensor)

    #MIPI CSI-2 auf 800 Mbps @ 25 MHz REFCLK -> @24 MHz bei 768 MHz
    i2cset -y -f $i2c_bus $FPDLINK_ADDR 0x1F 0x02  #800 Mbps / MIPI-CLK bei 400 MHz OK!
    i2cset -y -f $i2c_bus $FPDLINK_ADDR 0x33 0x01  #4 Lanes, enalbe CSI output

    #I2C Taktrate am Serializer auf ca. 400 kHz @ 25 MHz REFCLK
    i2cset -y -f $i2c_bus $serializer_addr 0x0b 0x17
    i2cset -y -f $i2c_bus $serializer_addr 0x0c 0x17

    i2cset -y -f $i2c_bus $FPDLINK_ADDR 0x20 0x20  #Port forwarding

    return 0
  else
    return -1
  fi
}

i2c_get_16bit() {
  local i2c_bus=$1
  local dev_addr=$2
  local reg_addr=$3
  local return=$4

  # set reg_address
  i2cset -y -f  $i2c_bus $dev_addr ${reg_addr::-2} ${reg_addr:0:2}${reg_addr:4}
  # echo $i2c_bus $dev_addr ${reg_addr::-2} ${reg_addr:0:2}${reg_addr:4}

  # read content at reg_address
  d0_h=`i2cget -y -f $i2c_bus $dev_addr` && d0_l=`i2cget -y -f $i2c_bus $dev_addr`

  # combine the 2 bytes
  a="0x${d0_h:2}${d0_l:2}"

  eval "$return=$a"
}

# TBD: rework to also detected devices that are used by a driver
check_i2c_device() {
  local i2c_bus="$1"
  local device_address="$2"
  local device_uid=$3
  local device_address_dir="${2//x/0}"
  
  local device_path="/sys/bus/i2c/devices/$i2c_bus-$device_address_dir"

  # check if device directory exists
  if [ -d "$device_path" ]; then
    # check if driver is loaded
    local driver_path="$device_path/driver"
    if [ -d "$driver_path" ]; then
      local driver=$(basename $(readlink "$driver_path"))
      echo "Driver is loaded"
      return 0 # device exists, driver is loaded
    fi
  fi
  # probe i2c device
  if i2cget -y $i2c_bus $device_address > /dev/null 2>&1; then
      if [[ -z "$device_uid" ]]; then # if no device uid is given (for fpdlink)
        return 1 # device exists, driver is not loaded
      else
        i2c_get_16bit $i2c_bus $device_address 0x3000 result
        if [ $result = $device_uid ]; then
          return 1 # device exists, driver is not loaded
        else
          return 2 # correct device_address, wrong device_uid -> device not found
        fi
      fi


  else
    echo "device not found"
    return 2 # device not found
  fi
}

set_overlays() {
  overlays_file="/boot/bootenv.txt"
    
  # remove all camera overlays from bootenv.txt
  for overlay in "${overlaysToRemove[@]}"; do
      sed -i "s/${overlay}//g" "$overlays_file"
      exit_code=$?
      
      if [ $exit_code -lt 0 ]; then
          echo "Error modifying ${overlays_file}"
          exit -1
      fi
  done

  sed -i "\$ s/\$/$(echo " $1")/" "${overlays_file}"

  reboot now
  exit 0
}

######################## GET OPTIONS ########################
while getopts "ms:" opt; do
  case $opt in
    m) # enable machine readable format
      human_readable=false
      ;;
    s) # set devicetree overlays and reboot
      set_overlays "$OPTARG"
      ;;
    \?) # TBD: optional
      echo "Invalid option: -$OPTARG" >&2
      exit -1
      ;;
  esac
done

######################## CSI1 ########################
fpdlinkConnected=0
check_i2c_device "$CSI_1_I2C_BUS" "$FPDLINK_ADDR" # FPDLINK
exit_code=$?
if [ $exit_code -lt 2 ]; then # device exists
  if [ "$human_readable" = true ]; then
    echo "Detected FPDLINK on CSI1"
  fi
  fpdlinkConnected=1
fi

for ((i = 0 ; i <= $fpdlinkConnected ; i++)); do
  if [ $fpdlinkConnected -eq 1 ]; then
    fpdlinkPort=$i

    enable_fpdlink "$CSI_1_I2C_BUS" "$fpdlinkPort"
  fi

  for (( i=0;  i < ${#CAMERA_NAMES[@]};  i++ )); do
    check_i2c_device "$CSI_1_I2C_BUS" ${CAMERA_ADDRS[$i]} ${CAMERA_UIDS[$i]}
    exit_code=$?
    if [ $exit_code -lt 2 ]; then # device exists
      if [ "$human_readable" = true ]; then
        echo "Detected ${CAMERA_NAMES[$i]} on CSI1"
      fi
      recommended_dtos+=("imx8mp-isi-csi1.dtbo")
      recommended_dtos+=("imx8mp-isp-csi1.dtbo") 

      if [ $fpdlinkConnected -eq 1 ]; then
        recommended_dtos+=("${CAMERA_OVERLAYS[$i]}-csi1-fpdlink-port${fpdlinkPort}.dtbo")
      else
        recommended_dtos+=("${CAMERA_OVERLAYS[$i]}-csi1.dtbo")
      fi
    fi
  done
done


######################## CSI2 ########################
fpdlinkConnected=0
check_i2c_device "$CSI_2_I2C_BUS" "$FPDLINK_ADDR" # FPDLINK
exit_code=$?
if [ $exit_code -lt 2 ]; then # device exists
  if [ "$human_readable" = true ]; then
    echo "Detected FPDLINK on CSI2"
  fi
  fpdlinkConnected=1
fi

for ((i = 0 ; i <= $fpdlinkConnected ; i++)); do
  if [ $fpdlinkConnected -eq 1 ]; then
    fpdlinkPort=$i

    enable_fpdlink "$CSI_2_I2C_BUS" "$fpdlinkPort"
  fi

  for (( i=0;  i < ${#CAMERA_NAMES[@]};  i++ )); do
    check_i2c_device "$CSI_2_I2C_BUS" ${CAMERA_ADDRS[$i]} ${CAMERA_UIDS[$i]}
    exit_code=$?
    if [ $exit_code -lt 2 ]; then # device exists
      if [ "$human_readable" = true ]; then
        echo "Detected ${CAMERA_NAMES[$i]} on CSI2"
      fi
      recommended_dtos+=("imx8mp-isi-csi2.dtbo")
      recommended_dtos+=("imx8mp-isp-csi2.dtbo") 

      if [ $fpdlinkConnected -eq 1 ]; then
        recommended_dtos+=("${CAMERA_OVERLAYS[$i]}-csi2-fpdlink-port${fpdlinkPort}.dtbo")
      else
        recommended_dtos+=("${CAMERA_OVERLAYS[$i]}-csi2.dtbo")
      fi
    fi
  done
done


if (( ${#recommended_dtos[@]} )); then
  # for recommended_dto in "${recommended_dtos[@]}"; do # Print recommended dtos
  if [ "$human_readable" = true ]; then
    echo "Try loading the following overlays by adding them to /boot/bootenv.txt and reboot"
    echo ""
    echo "${recommended_dtos[*]}"

    echo "Do you want to load the overlays and reboot? (y/n)"

    # Benutzereingabe abfragen
    read input

    # Überprüfen, ob die Eingabe "y" oder "Y" ist, um den Code auszuführen
    if [ "$input" = "y" ] || [ "$input" = "Y" ]|| [ "$input" = "" ]; then
      set_overlays "${recommended_dtos[*]}"
    else
      exit 0
    fi
  else
    echo "${recommended_dtos[*]}"
    exit 0
  fi
else
  if [ "$human_readable" = true ]; then
    echo "No Camera found on the CSI interfaces!"
  fi
  exit 1
fi