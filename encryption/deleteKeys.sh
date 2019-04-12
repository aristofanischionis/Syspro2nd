#!/bin/bash
# first delete secret keys
for i in `gpg --with-colons --fingerprint | grep "^fpr" | cut -d: -f10`; do gpg --batch --delete-secret-keys "$i" ; done
# delete public keys
for i in `gpg --with-colons --fingerprint | grep "^fpr" | cut -d: -f10`; do gpg --batch --delete-key "$i" ; done
