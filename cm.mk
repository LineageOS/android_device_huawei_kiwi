$(call inherit-product, device/qrd/cp8675/full_cp8675.mk)

# Inherit some common CM stuff.
$(call inherit-product, vendor/cm/config/common_full_phone.mk)

# Enhanced NFC
$(call inherit-product, vendor/cm/config/nfc_enhanced.mk)

PRODUCT_RELEASE_NAME := Coolpad 8675
PRODUCT_NAME := cm_cp8675
