from base64 import b64encode, b64decode

hex = "40CFEDE10100010001BB0B3BDFDE23B27F669A3C365F697F538A14B1C2"
b64 = b64encode(bytes.fromhex(hex)).decode()
print(b64)

