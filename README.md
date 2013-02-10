# RANDTEST

A simple suit of randomness tests for bit sequences, licensed under a BSD 3 clause style license (check LICENSE.md file).

## Operation

randtest [ -p | -u | -t ] sequencefile *reportfile*

* -p **packed mode** treats each sequencefile byte as 8 bits of sequence to be tested
* -u **unpacked mode** treats each sequencefile byte as a single bit of sequence to be tested
* -t **text mode** expects each sequencefile byte to be either char '0' or char '1'

The sequence file is mandatory; if no report file is given, the report is printed on standard output.

## Additional Comments

This might need some work still. I think it works OK for not very long sequences. Take in mind that it will take some time for moderately long sequences.

If you want to find more about randomness testing please read ["Handbook of Applied Cryptography" by Menezes, Van Oorschot, and Vanstone](http://cacr.uwaterloo.ca/hac/)
