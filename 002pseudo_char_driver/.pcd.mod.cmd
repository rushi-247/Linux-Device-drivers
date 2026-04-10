savedcmd_pcd.mod := printf '%s\n'   pcd.o | awk '!x[$$0]++ { print("./"$$0) }' > pcd.mod
