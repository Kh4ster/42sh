case * in
toto) echo bad;;
$(echo *)) echo bad;;
*) echo good
esac

case $(echo *) in
toto) echo bad;;
$(echo *)) echo good;;
*) echo bad
esac
