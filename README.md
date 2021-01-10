# OneTime Pads

This is a step-by-step tutorial on how to run the OneTime Pads program.
-----
How to compile:
./compileall

Running the server:
./enc_server RANDOM_PORT1 &
./dec_server RANDOM_PORT2 &

Generate key:
./keygen SIZE > keyFileName

Starting the client/Encrypting the message:
./enc_client textToEncrypt keyFileName RANDOM_PORT1 > cipherTextFile

Decrypting the message:
./dec_client cipherTextFile keyFileName RANDOM_PORT2 > finalResults

Variables Guide:
RANDOM_PORT1 = Port Number for ENC
RANDOM_PORT2 = Port Number for DEC
keyFileName = The name you want the key file to be
textToEncrypt =  The text that you want to encrypt
cipherTextFile = Encrypted text
finalResults = Decrypted text
