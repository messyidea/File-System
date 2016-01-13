# File-System
A simulation of file system

support operations as follows:
```
mkdir:          create a empty dir
rmdir:          rm a empty
mv:             move file, you can also move dir using '-r' options
ls:             list files and dirs
touch:          create empty file
rm:             delete file, you can also delete dir using  '-r' options
cp:             copy file, you can also copy dir using '-r' options
cat:            read file and show the content
write:          write to file
cd:             chdir
useradd:        add a user
userdel:        delete user
su:             change user
passwd:         change password of user
du:             show the size of file or dir
tree:           show the file tree
chmod:          change the authority of file or dir
exit:           exit
```
you can also use 'command?' to know more info about this command.

display
---
```
root:/root#ls
drwxr-xr-x          99        root           root           Wed Jan 13 13:10:16 2016      .              
drwxr-xr-x          0         root           root           Wed Jan 13 13:10:16 2016      ..             
root:/root#cd /
root:/#ls
drwxr-xr-x          0         root           root           Wed Jan 13 13:10:16 2016      .              
drwxr-xr-x          0         root           root           Wed Jan 13 13:10:16 2016      ..             
drwxr-xr-x          100       root           root           Wed Jan 13 13:10:16 2016      etc            
drwxr-xr-x          99        root           root           Wed Jan 13 13:10:16 2016      root           
drwxr-xr-x          98        root           root           Wed Jan 13 13:10:16 2016      home           
root:/#cd root
root:/root#mkdir a b
root:/root#cd a
root:/root/a#mkdir c
root:/root/a#cd c
root:/root/a/c#touch d
root:/root/a/c#cd ../../
root:/root#ls
drwxr-xr-x          99        root           root           Wed Jan 13 13:10:16 2016      .              
drwxr-xr-x          0         root           root           Wed Jan 13 13:10:16 2016      ..             
drwxr-xr-x          96        root           root           Wed Jan 13 13:18:15 2016      a              
drwxr-xr-x          95        root           root           Wed Jan 13 13:18:15 2016      b              
root:/root#tree
/root
|── a
|   |── c
|       |── d
|── b
root:/root#cp a b -r
root:/root#tree
/root
|── a
|   |── c
|       |── d
|── b
    |── a
        |── c
            |── d
root:/root#cp a c -0r
cp: 参数错误
root:/root#cp a c -r
root:/root#tree
/root
|── a
|   |── c
|       |── d
|── b
|   |── a
|       |── c
|           |── d
|── c
    |── c
        |── d
root:/root#^C
wait for save data
save ok
```
