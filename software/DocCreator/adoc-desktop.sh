 #!/bin/sh
cat <<EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=DocCreator
GenericName=Editor
Comment=Create old and degraded documents
TryExec=$1/bin/DocCreator
Exec=$1/bin/DocCreator &
Categories=Graphics;
Icon=$1/share/icons/adoc.icns
EOF
