echo cool > toto
while cat toto; do rm toto; done
