savedcmd_pcd_n.mod := printf '%s\n'   pcd_n.o | awk '!x[$$0]++ { print("./"$$0) }' > pcd_n.mod
