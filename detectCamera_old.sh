#!/bin/bash

CSI_1_I2C_BUS=2
CSI_2_I2C_BUS=1

VM016_ADDR=0x10
VM017_ADDR=0x36 
FPDLINK_ADDR=0x3d

BOOTENV_FILE="/boot/bootenv.txt"

expected_dtos=()
fpdlinkConnected=0

check_i2c_device_old() {
  local i2c_bus="$1"
  local device_address="$2"
  local device_address_dir="${2//x/0}"
  
  local device_path="/sys/bus/i2c/devices/$i2c_bus-$device_address_dir"

  # check if device directory exists
  if [ -d "$device_path" ]; then
    # check if driver is loaded
    local driver_path="$device_path/driver"
    if [ -d "$driver_path" ]; then
      local driver=$(basename $(readlink "$driver_path"))
      return 0 # device exists, driver is loaded
    else
      return 1 # device exists, driver is not loaded
    fi
  else
    return 2 # device not found
  fi
}

check_i2c_device() {
  local i2c_bus="$1"
  local device_address="$2"
  local device_address_dir="${2//x/0}"
  
  local device_path="/sys/bus/i2c/devices/$i2c_bus-$device_address_dir"

  # probe i2c device

  if i2cget -y $i2c_bus $device_address > /dev/null 2>&1; then
    # check if device directory exists
    if [ -d "$device_path" ]; then
      # check if driver is loaded
      local driver_path="$device_path/driver"
      if [ -d "$driver_path" ]; then
        local driver=$(basename $(readlink "$driver_path"))
        return 0 # device exists, driver is loaded
      fi
    else
      return 1 # device exists, driver is not loaded
    fi
  else
    return 2 # device not found
  fi
}

# Check CSI1
check_i2c_device "$CSI_1_I2C_BUS" "$FPDLINK_ADDR" # FPDLINK
exit_code=$?
if [ $exit_code -lt 2 ]; then # device exists
  echo "Detected FPDLINK on CSI1"
  expected_dtos+=("imx8mp-isi-csi1.dtbo")
  expected_dtos+=("imx8mp-isp-csi1.dtbo") 
  expected_dtos+=("imx8mp-<YOUR_CAMERA>-csi1-fpdlink-port<YOUR_FPDLINK_PORT>.dtbo")
  fpdlinkConnected=1
fi

check_i2c_device "$CSI_1_I2C_BUS" "$VM016_ADDR" # VM016
exit_code=$?
if [ $exit_code -lt 2 ]; then # device exists
  echo "Detected VM016 on CSI1"
  expected_dtos+=("imx8mp-isi-csi1.dtbo")
  expected_dtos+=("imx8mp-isp-csi1.dtbo") 
  expected_dtos+=("imx8mp-vm016-csi1.dtbo")
fi
check_i2c_device "$CSI_1_I2C_BUS" "$VM017_ADDR" # VM017
exit_code=$?
if [ $exit_code -lt 2 ]; then # device exists
  echo "Detected VM017 on CSI1"
  expected_dtos+=("imx8mp-isi-csi1.dtbo")
  expected_dtos+=("imx8mp-isp-csi1.dtbo") 
  expected_dtos+=("imx8mp-vm017-csi1.dtbo")
fi



# Check CSI2
check_i2c_device "$CSI_2_I2C_BUS" "$FPDLINK_ADDR" # FPDLINK
exit_code=$?
if [ $exit_code -lt 2 ]; then # device exists
  echo "Detected FPDLINK on CSI2"
  expected_dtos+=("imx8mp-isi-csi2.dtbo")
  expected_dtos+=("imx8mp-isp-csi2.dtbo") 
  expected_dtos+=("imx8mp-<YOUR_CAMERA>-csi2-fpdlink-port<YOUR_FPDLINK_PORT>.dtbo")
  fpdlinkConnected=1
fi

check_i2c_device "$CSI_2_I2C_BUS" "$VM016_ADDR" # VM016
exit_code=$?
if [ $exit_code -lt 2 ]; then # device exists
  echo "Detected VM016 on CSI2"
  expected_dtos+=("imx8mp-isi-csi2.dtbo")
  expected_dtos+=("imx8mp-isp-csi2.dtbo") 
  expected_dtos+=("imx8mp-vm016-csi2.dtbo")
fi
check_i2c_device "$CSI_2_I2C_BUS" "$VM017_ADDR" # VM017
exit_code=$?
if [ $exit_code -lt 2 ]; then # device exists
  echo "Detected VM017 on CSI2"
  expected_dtos+=("imx8mp-isi-csi2.dtbo")
  expected_dtos+=("imx8mp-isp-csi2.dtbo") 
  expected_dtos+=("imx8mp-vm017-csi2.dtbo")
fi


if (( ${#expected_dtos[@]} )); then
  # for expedted_dto in "${expedted_dtos[@]}"; do # Print expected dtos
  echo "Try loading the following overlays by adding them to /boot/bootenv.txt and reboot"
  echo ""
  echo "${expected_dtos[*]}"
  if [ $fpdlinkConnected -eq 0 ]; then
    exit 1
  else
    exit 2
  fi
else
  echo "No Camera found on the CSI interfaces!"
  exit 3
fi



# # Check loaded overlays
# if [ -e "$BOOTENV_FILE" ]; then
#   trimmed_content=$(sed -n 's/^overlays=//p' "$BOOTENV_FILE") # cut away leading "overlays="
#   dtos=($trimmed_content)

#   for dto in "${dtos[@]}"; do # iterate over dtos in bootenv.txt
#     echo "$dto"
#   done
# else
#   echo "ERROR: the file $BOOTENV_FILE does not exist!"
# fi






# exit_code=$?
# if [ $exit_code -eq 0 ]; then # device exists, driver is loaded
#   echo "Gerät ist vorhanden und ein Treiber wird verwendet."
# elif [ $exit_code -eq 1 ]; then # device exists, driver is not loaded
#   echo "Gerät ist vorhanden, aber es wird kein Treiber verwendet."
# else # device not found
#   echo "Gerät nicht gefunden."
# fi