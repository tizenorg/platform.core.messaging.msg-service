sed -n '1p' ./debian/changelog  | cut -f2 -d "(" | cut -f1 -d ")"
