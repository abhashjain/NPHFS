// Project 3: Jithin John, jjohn3; Abhash Jain, ajain28; Bhavya Bansal, bbansal; 
/*
  NPHeap File System
  Copyright (C) 2016 Hung-Wei Tseng, Ph.D. <hungwei_tseng@ncsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  This code is derived from function prototypes found /usr/include/fuse/fuse.h
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  His code is licensed under the LGPLv2.
  A copy of that code is included in the file fuse.h
  
  The point of this FUSE filesystem is to provide an introduction to
  FUSE.  It was my first FUSE filesystem as I got to know the
  software; hopefully, the comments in this code will help people who
  follow later to get a gentler introduction.

*/

#include "nphfuse.h"
#include <npheap.h>

extern struct nphfuse_state* nphfuse_data;

///////////////////////////////////////////////////////////
//
// Prototypes for all these functions, and the C-style comments,
// come from /usr/include/fuse.h
//
/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
int nphfuse_getattr(const char *path, struct stat *stbuf)
{
    log_msg("\nnphfuse_getattr: path: %s , statbuf: %x\n",path,stbuf);
    if(!strcmp(path,ROOT_DIR))
    {
	log_msg("\nnphfuse_getattr: inside root directory\n");
	npheap_lock(nphfuse_data->devfd, ROOT_INODE);                          
        void* root = npheap_alloc(nphfuse_data->devfd, ROOT_INODE, BLOCK_SIZE);
	memset(stbuf,0,sizeof(struct stat));
	memcpy(stbuf,&(((struct nphfuse_dir*)root)->myStat),sizeof(struct stat));
	npheap_unlock(nphfuse_data->devfd, ROOT_INODE);
	log_stat(stbuf);
	log_msg("\nAbhash is returning root\n");
	return 0;   //testing to be changed to SUCCESS
    }
    char* tok = strtok(path,"/");
    int currentNode = ROOT_INODE;
//    tok = strtok(NULL,"/");
    while(tok!=NULL)
    {
    	npheap_lock(nphfuse_data->devfd, currentNode);                          
	void* cwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
	int i=0;
	int found = 0;
	for(i=0;i<DIRENTNUM;i++)
	{
            if(!((struct nphfuse_dir*)cwd)->direntList[i].d_fileno)
                continue;
	    if(!strcmp(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok))
	    {
		npheap_unlock(nphfuse_data->devfd,currentNode);
		currentNode = ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno;
		found = 1;
		break;
	    }
	}
	if(!found){
	    log_msg("nphfuse_getattr: Inode not found\n");
	    npheap_unlock(nphfuse_data->devfd,currentNode);
	    //return empty statbuf
	    memset(stbuf,0,sizeof(struct stat));
	    log_stat(stbuf);
	    return -ENOENT;
	}
	tok = strtok(NULL,"/");
	
     }
     log_msg("nphfuse_getattr: returning a new node with crtNode %d\n",currentNode);
     npheap_lock(nphfuse_data->devfd, currentNode);
     void* fwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
     memset(stbuf,0,sizeof(struct stat));
     memcpy(stbuf,&((((struct nphfuse_dir*)(fwd)))->myStat),sizeof(struct stat));
     npheap_unlock(nphfuse_data->devfd, currentNode);
     log_stat(stbuf);
    return 0; //changed for testing
    
}

/** Read the target of a symbolic link
 *
 * The buffer should be filled with a null terminated string.  The
 * buffer size argument includes the space for the terminating
 * null character.  If the linkname is too long to fit in the
 * buffer, it should be truncated.  The return value should be 0
 * for success.
 */
// Note the system readlink() will truncate and lose the terminating
// null.  So, the size passed to to the system readlink() must be one
// less than the size passed to nphfuse_readlink()
// nphfuse_readlink() code by Bernardo F Costa (thanks!)
int nphfuse_readlink(const char *path, char *link, size_t size)
{
    log_msg("abhash nphfuse_readlink\n");
    return -1;
}

/** Create a file node
 *
 * There is no create() operation, mknod() will be called for
 * creation of all non-directory, non-symlink nodes.
 */
int nphfuse_mknod(const char *path, mode_t mode, dev_t dev)
{
    log_msg("nphfuse_mknod: for path: %s \n",path);
   char* tok = strtok(path,"/");
    int currentNode = ROOT_INODE;
//    tok = strtok(NULL,"/");
    while(tok!=NULL)
    {
        npheap_lock(nphfuse_data->devfd, currentNode);       
        void* cwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
        int i=0;
        int found = 0;
        for(i=0;i<DIRENTNUM;i++)
        {
            if(!((struct nphfuse_dir*)cwd)->direntList[i].d_fileno)
                continue;
            if(!strcmp(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok))
            {
		log_msg("nphfuse_mknod: node found on offset %d\n",currentNode);
                npheap_unlock(nphfuse_data->devfd,currentNode);
                currentNode = ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno;
                found = 1;
                break;
            }
	   /* if(((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno == 0) {
		npheap_unlock(nphfuse_data->devfd,currentNode);
		break;
	    }*/
        }
	npheap_unlock(nphfuse_data->devfd,currentNode);
        if(!found){
	    if(strtok(NULL,"/")!= NULL){
            	npheap_unlock(nphfuse_data->devfd,currentNode);
            	return -ENOENT;
	    }
	    else {
		log_msg("nphfuse_mknod: Node not found. to be created to be currentNode = %d\n",currentNode);
		int j =1;
		int emptyIndex = 1;
		for(j = 1; j<8192;j++)
		{
		    log_msg("nphfuse_mknod: j=%d\n",j);
		    if(j == currentNode){
			continue;
		    }
		    log_msg("\nnphfuse_mknod: abhash mknod for j=%d currentNode= %d\n",j,currentNode);
		    npheap_lock(nphfuse_data->devfd,j);
		    int size = npheap_getsize(nphfuse_data->devfd,j);
		    npheap_unlock(nphfuse_data->devfd,j);
		    if(size == 0) {
			emptyIndex = j;
			log_msg("\nnphfuse_mknod: abhash got free block at j= %d and current Node %d is\n",j,currentNode);
			break;
		    }
		}
		log_msg("\nnphfuse_mknod: abhash after for loop\n");
		if(j == 8192){
		    //npheap_unlock(nphfuse_data->devfd,currentNode);
		    log_msg("\nnphfuse_mknod NPHEAP full\n");
		    return -ENOENT;
		}
		else {
		   log_msg("\nnphfuse_mknod Abhash :got emptyIndex %d\n",emptyIndex);
		   for(i=0;i<DIRENTNUM;i++) {
			if(!((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno)
				break;
		   }
		   if(i==DIRENTNUM) {
		   	log_msg("\nnphfuse_mknod Folder full\n");
			npheap_unlock(nphfuse_data->devfd,currentNode);
		   	return -ENOMEM; 
		   }
		   log_msg("\nnphfuse_mknod: creating a new node at emptyIndex %d for %s\n and updating parent at %d\n",emptyIndex,tok,i);
		    npheap_lock(nphfuse_data->devfd,currentNode);
		   ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno = emptyIndex;
		   ((struct nphfuse_dir*)(cwd))->myStat.st_nlink++;
		   strcpy(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok);
		   //((struct nphfuse_dir*)(cwd))->direntList[i].d_type = DT_DIR;
		   ((struct nphfuse_dir*)(cwd))->myStat.st_mtime = time(NULL);
		   npheap_unlock(nphfuse_data->devfd,currentNode);
		   struct nphfuse_dir newDir;
		   newDir.myStat.st_ino = emptyIndex;
		   newDir.myStat.st_atime = time(NULL);
		   newDir.myStat.st_ctime = time(NULL);
		   newDir.myStat.st_mtime = time(NULL);
		   newDir.myStat.st_blksize = BLOCK_SIZE;
		   newDir.myStat.st_size = 0;	//Abhash: thinking new node size will be 0
		   newDir.myStat.st_uid = getuid(); 
		   newDir.myStat.st_gid = getgid(); 
		   newDir.myStat.st_nlink = 1;
        	   newDir.myStat.st_rdev = 0;
        	   newDir.myStat.st_dev = nphfuse_data->devfd;
		   newDir.myStat.st_mode = mode;
		   int nextEmptyIndex = emptyIndex + 1;
		   for(;nextEmptyIndex<8192;nextEmptyIndex++)
		   {
		   	npheap_lock(nphfuse_data->devfd,nextEmptyIndex);
		    	int size = npheap_getsize(nphfuse_data->devfd,nextEmptyIndex);
		    	npheap_unlock(nphfuse_data->devfd,nextEmptyIndex);
		    	if(size == 0) {
				break;
		    	}
		   }
		   if(nextEmptyIndex == 8192)
		   {
			log_msg("\nnphfuse_mknod: NPHEAP full\n");
			return -ENOMEM;
		   }
		   
		   void *data_ptr = npheap_alloc(nphfuse_data->devfd,nextEmptyIndex,BLOCK_SIZE);
		   memset(data_ptr,0,BLOCK_SIZE);
		   npheap_unlock(nphfuse_data->devfd,nextEmptyIndex);
		   newDir.direntList[0].d_fileno = nextEmptyIndex;
		   //npheap_lock(nphfuse_data->devfd,emptyIndex);
		   void* dirPtr = npheap_alloc(nphfuse_data->devfd,emptyIndex,BLOCK_SIZE);
		   memset(dirPtr,0,BLOCK_SIZE);
		   memcpy(dirPtr,&newDir,sizeof(newDir));
		   npheap_unlock(nphfuse_data->devfd,emptyIndex);
		   log_msg("nphfuse_mknod: returning from mknod\n");
		   return 0;	//changed for testing
		}
	    }
        }
	else {
	     npheap_unlock(nphfuse_data->devfd,currentNode);
	}
        	tok = strtok(NULL,"/"); 
    } 
    return -ENOENT;
}

/** Create a directory */
int nphfuse_mkdir(const char *path, mode_t mode)
{
    log_msg("\nnphfuse_mkdir Inside: %s\n",path);
    char* tok = strtok(path,"/");
    int currentNode = ROOT_INODE;
//    tok = strtok(NULL,"/");
    while(tok!=NULL)
    {
        npheap_lock(nphfuse_data->devfd, currentNode);       
        void* cwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
        int i=0;
        int found = 0;
        for(i=0;i<DIRENTNUM;i++)
        {
            if(!((struct nphfuse_dir*)cwd)->direntList[i].d_fileno)
                continue;
            if(!strcmp(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok))
            {
		log_msg("nphfuse_mkdir: node found on offset %d\n",currentNode);
                npheap_unlock(nphfuse_data->devfd,currentNode);
                currentNode = ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno;
                found = 1;
                break;
            }
	   /* if(((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno == 0) {
		npheap_unlock(nphfuse_data->devfd,currentNode);
		break;
	    }*/
        }
	npheap_unlock(nphfuse_data->devfd,currentNode);
        if(!found){
	    if(strtok(NULL,"/")!= NULL){
            	npheap_unlock(nphfuse_data->devfd,currentNode);
            	return -ENOENT;
	    }
	    else {
		log_msg("nphfuse_mkdir: Node not found. to be created\n");
		int j =1;
		int emptyIndex = 1;
		for(j = 1; j<8192;j++)
		{
		    if(j == currentNode)
			continue;
		    npheap_lock(nphfuse_data->devfd,j);
		    int size = npheap_getsize(nphfuse_data->devfd,j);
		    npheap_unlock(nphfuse_data->devfd,j);
		    if(size == 0) {
			emptyIndex = j;
			break;
		    }
		}
		if(j == 8192){
		    npheap_unlock(nphfuse_data->devfd,currentNode);
		    return -ENOENT;
		}
		else {
		   for(i=0;i<DIRENTNUM;i++) {
			if(!((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno)
				break;
		   }
		   if(i==DIRENTNUM) {
		   	log_msg("\nFolder full\n");
			npheap_unlock(nphfuse_data->devfd,currentNode);
		   	return -ENOMEM; 
		   }
		   log_msg("nphfuse_mkdir: creating a new node at emptyIndex %d for %s\n and updating parent at %d\n",emptyIndex,tok,i);
		   npheap_lock(nphfuse_data->devfd,currentNode);
		   ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno = emptyIndex;
		   strcpy(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok);
		   //((struct nphfuse_dir*)(cwd))->direntList[i].d_type = DT_DIR;
		   ((struct nphfuse_dir*)(cwd))->myStat.st_mtime = time(NULL);
		   npheap_unlock(nphfuse_data->devfd,currentNode);
		   struct nphfuse_dir newDir;
		   newDir.myStat.st_ino = emptyIndex;
		   newDir.myStat.st_atime = time(NULL);
		   newDir.myStat.st_ctime = time(NULL);
		   newDir.myStat.st_mtime = time(NULL);
		   newDir.myStat.st_blksize = BLOCK_SIZE;
		   newDir.myStat.st_size = BLOCK_SIZE;
		   newDir.myStat.st_uid = getuid(); 
		   newDir.myStat.st_gid = getgid(); 
		   newDir.myStat.st_nlink = 2;
        	   newDir.myStat.st_rdev = 0;
        	   newDir.myStat.st_dev = nphfuse_data->devfd;
		   newDir.myStat.st_mode = mode | S_IFDIR;
		   int x = 0;
		   for(x=0;x<DIRENTNUM;x++)
			newDir.direntList[x].d_fileno = 0;
		   npheap_lock(nphfuse_data->devfd,emptyIndex);
		   void* dirPtr = npheap_alloc(nphfuse_data->devfd,emptyIndex,BLOCK_SIZE);
		   memset(dirPtr,0,BLOCK_SIZE);
		   memcpy(dirPtr,&newDir,sizeof(newDir));
		   npheap_unlock(nphfuse_data->devfd,emptyIndex);
		   log_msg("nphfuse_mkdir: returning from mkdir\n");
		   return 0;	//changed for testing
		}
	    }
        }
	else {
	     npheap_unlock(nphfuse_data->devfd,currentNode);
	}
        	tok = strtok(NULL,"/");
        
     }
	    
	return -ENOENT;
}

	/** Remove a file */
int nphfuse_unlink(const char *path)
{
	log_msg("nphfuse_unlink for path %s\n",path);
    char* tok = strtok(path,"/");
    int currentNode = ROOT_INODE;
    int deleteDirIndex = -1;
    void* cwd = NULL;
    while(tok!=NULL)
    {
        npheap_lock(nphfuse_data->devfd, currentNode);
        cwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
        int i=0;
        int found = 0;
        for(i=0;i<DIRENTNUM;i++)
        {
            if(!((struct nphfuse_dir*)cwd)->direntList[i].d_fileno)
                continue;
            if(!strcmp(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok))
            {
                npheap_unlock(nphfuse_data->devfd,currentNode);
                currentNode = ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno;
                found = 1;
                deleteDirIndex = i;
                break;
            }
        }
        if(!found){
            log_msg("nphfuse_unlink: Inode not found\n");
            npheap_unlock(nphfuse_data->devfd,currentNode);
            //return empty statbuf
            return -ENOENT;
        }
        tok = strtok(NULL,"/");
     }
    if(!cwd || deleteDirIndex == -1){
        log_msg("\nnphfuse_unlink: Node not found\n");
        return -ENOENT;
     }
     log_msg("\nnphfuse_unlink: updating dirent from parent at index: %d\n",deleteDirIndex);
	 int file_Index = ((struct nphfuse_dir*)(cwd))->direntList[deleteDirIndex].d_fileno;
	 npheap_lock(nphfuse_data->devfd, file_Index);
	 void* file_inode = npheap_alloc(nphfuse_data->devfd,file_Index,BLOCK_SIZE);
	((struct nphfuse_dir*)(cwd))->direntList[deleteDirIndex].d_fileno = 0;
	 if(((struct nphfuse_dir*)(file_inode))->myStat.st_nlink ==1){
	 	//delete the node and data 
		int data_ino = ((struct nphfuse_dir*)(file_inode))->direntList[0].d_fileno;
		log_msg("nphfuse_unlink: data inode number is %d\n",data_ino);
		npheap_unlock(nphfuse_data->devfd, file_Index);
		npheap_lock(nphfuse_data->devfd,data_ino);
		npheap_delete(nphfuse_data->devfd,data_ino);
		npheap_unlock(nphfuse_data->devfd,data_ino);
		npheap_lock(nphfuse_data->devfd, file_Index);
		npheap_delete(nphfuse_data->devfd,file_Index);
		npheap_unlock(nphfuse_data->devfd, file_Index);
		//update the parent direecty dirent
     	log_msg("nphfuse_unlink: deleting node with inode no:  %d\n",currentNode);
     	npheap_lock(nphfuse_data->devfd, currentNode);
     	npheap_delete(nphfuse_data->devfd, currentNode);
     	npheap_unlock(nphfuse_data->devfd, currentNode);
	} else{
		//update the count
		((struct nphfuse_dir*)(file_inode))->myStat.st_nlink--;
		npheap_unlock(nphfuse_data->devfd, file_Index);
	}
		log_msg("nphfuse_unlink: File is deleted\n");
        return 0;
	}

	/** Remove a directory */
int nphfuse_rmdir(const char *path)
{
	log_msg("\nnphfuse_rmdir\n");
	char* tok = strtok(path,"/");
    int currentNode = ROOT_INODE;
    int deleteDirIndex = -1;
    void* cwd = NULL;
    while(tok!=NULL)
    {   
        npheap_lock(nphfuse_data->devfd, currentNode);                         
        cwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
        int i=0;
        int found = 0;
        for(i=0;i<DIRENTNUM;i++)
        {   
            if(!((struct nphfuse_dir*)cwd)->direntList[i].d_fileno)
                continue;
            if(!strcmp(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok))
            {   
                npheap_unlock(nphfuse_data->devfd,currentNode);
                currentNode = ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno;
                found = 1;
				deleteDirIndex = i;
                break;
            }
        }
        if(!found){
            log_msg("nphfuse_rmdir: Inode not found\n");
            npheap_unlock(nphfuse_data->devfd,currentNode);
            //return empty statbuf
            return -ENOENT;
        }
        tok = strtok(NULL,"/");
      
     }
	if(!cwd || deleteDirIndex == -1){
	    log_msg("\nnphfuse_rmdir: Node not found\n");
	    return -ENOENT;
     }
     log_msg("\nnphfuse_rmdir: removing dirent from parent at index: %d\n",deleteDirIndex);
     ((struct nphfuse_dir*)(cwd))->direntList[deleteDirIndex].d_fileno = 0;
     log_msg("nphfuse_rmdir: deleting node with inode no:  %d\n",currentNode);
     npheap_lock(nphfuse_data->devfd, currentNode);
     void* fwd = npheap_delete(nphfuse_data->devfd, currentNode);
     npheap_unlock(nphfuse_data->devfd, currentNode);

	    return 0;
}

	/** Create a symbolic link */
	// The parameters here are a little bit confusing, but do correspond
	// to the symlink() system call.  The 'path' is where the link points,
	// while the 'link' is the link itself.  So we need to leave the path
	// unaltered, but insert the link into the mounted directory.
	int nphfuse_symlink(const char *path, const char *link)
	{
	    log_msg("nphfuse_symlink\n");
	    return -1;
	}

	/** Rename a file */
	// both path and newpath are fs-relative
int nphfuse_rename(const char *path, const char *newpath)
{
    log_msg("nphfuse_rename: with existing file path: %s and new file path: %s\n",path,newpath);
    char* tok = strtok(path,"/");
    int currentNode = ROOT_INODE;
	void* src=NULL;
	int srcIndex=0;
    int srcNode =0;
//    tok = strtok(NULL,"/");
    while(tok!=NULL)
    {
        npheap_lock(nphfuse_data->devfd, currentNode);
        void* cwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
		src=cwd;
		int i=0;
        int found = 0;
        for(i=0;i<DIRENTNUM;i++)
        {
			srcIndex =i;
            if(!((struct nphfuse_dir*)cwd)->direntList[i].d_fileno)
                continue;
            if(!strcmp(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok))
            {
                npheap_unlock(nphfuse_data->devfd,currentNode);
				srcNode = currentNode;
                currentNode = ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno;
                found = 1;
                break;
            }
        }
        if(!found){
            log_msg("nphfuse_rename: Inode not found\n");
            npheap_unlock(nphfuse_data->devfd,currentNode);
            //return empty statbuf
            return -ENOENT;
        }
        tok = strtok(NULL,"/");

     }
     log_msg("nphfuse_rename: found file to be moved with inode number: %d\n",currentNode);
     npheap_lock(nphfuse_data->devfd, srcNode);
     void* fwd = npheap_alloc(nphfuse_data->devfd, srcNode , BLOCK_SIZE);
     ((struct nphfuse_dir*)(fwd))->direntList[srcIndex].d_fileno = 0;
     //memset(stbuf,0,sizeof(struct stat));
     //memcpy(stbuf,&((((struct nphfuse_dir*)(fwd)))->myStat),sizeof(struct stat));
     npheap_unlock(nphfuse_data->devfd, srcNode);

    int linkedNode = currentNode;

    tok = strtok(newpath,"/");
    currentNode = ROOT_INODE;
    while(tok!=NULL)
    {
        npheap_lock(nphfuse_data->devfd, currentNode);
        void* cwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
        int i=0;
        int found = 0;
        for(i=0;i<DIRENTNUM;i++)
        {
            if(!((struct nphfuse_dir*)cwd)->direntList[i].d_fileno)
                continue;
            if(!strcmp(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok))
            {
        log_msg("nphfuse_rename: intermediate node found on offset %d\n",currentNode);
                npheap_unlock(nphfuse_data->devfd,currentNode);
                currentNode = ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno;
                found = 1;
                break;
            }
       /* if(((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno == 0) {
        npheap_unlock(nphfuse_data->devfd,currentNode);
        break;
        }*/
        }
        if(!found){
        if(strtok(NULL,"/")!= NULL){
                npheap_unlock(nphfuse_data->devfd,currentNode);
                return -ENOENT;
        }
        else {
        log_msg("nphfuse_rename: Node not found. to be created\n");
           for(i=0;i<DIRENTNUM;i++) {
            if(!((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno)
                break;
           }
           if(i==DIRENTNUM) {
            log_msg("\nFolder full\n");
            npheap_unlock(nphfuse_data->devfd,currentNode);
            return -ENOMEM;
           }
           log_msg("nphfuse_rename: creating a new link node Node number %d with name: %s and updating parent at %d\n",linkedNode,tok,i);
           ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno = linkedNode;
           strcpy(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok);
           //((struct nphfuse_dir*)(cwd))->direntList[i].d_type = DT_DIR;
           ((struct nphfuse_dir*)(cwd))->myStat.st_mtime = time(NULL);
           npheap_unlock(nphfuse_data->devfd,currentNode);
           log_msg("nphfuse_rename: returning from mkdir\n");
           return 0;    //changed for testing

        }
        }
    else {
         npheap_unlock(nphfuse_data->devfd,currentNode);
    }
            tok = strtok(NULL,"/");

     }
    return -ENOENT;
	}

	/** Create a hard link to a file */
int nphfuse_link(const char *path, const char *newpath)
{
	log_msg("nphfuse_link: with existing file path: %s and new file path: %s\n",path,newpath);
    char* tok = strtok(path,"/");
    int currentNode = ROOT_INODE;
//    tok = strtok(NULL,"/");
    while(tok!=NULL)
    {
        npheap_lock(nphfuse_data->devfd, currentNode);
        void* cwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
        int i=0;
        int found = 0;
        for(i=0;i<DIRENTNUM;i++)
        {
            if(!((struct nphfuse_dir*)cwd)->direntList[i].d_fileno)
                continue;
            if(!strcmp(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok))
            {
                npheap_unlock(nphfuse_data->devfd,currentNode);
                currentNode = ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno;
                found = 1;
                break;
            }
        }
        if(!found){
            log_msg("nphfuse_link: Inode not found\n");
            npheap_unlock(nphfuse_data->devfd,currentNode);
            //return empty statbuf
            return -ENOENT;
        }
        tok = strtok(NULL,"/");

     }
     log_msg("nphfuse_link: found file to be linked with inode number: %d\n",currentNode);
     npheap_lock(nphfuse_data->devfd, currentNode);
     void* fwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
     ((struct nphfuse_dir*)(fwd))->myStat.st_nlink++;
     //memset(stbuf,0,sizeof(struct stat));
     //memcpy(stbuf,&((((struct nphfuse_dir*)(fwd)))->myStat),sizeof(struct stat));
     npheap_unlock(nphfuse_data->devfd, currentNode);

    int linkedNode = currentNode;

    tok = strtok(newpath,"/");
    currentNode = ROOT_INODE;
    while(tok!=NULL)
    {
        npheap_lock(nphfuse_data->devfd, currentNode);
        void* cwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
        int i=0;
        int found = 0;
        for(i=0;i<DIRENTNUM;i++)
        {
            if(!((struct nphfuse_dir*)cwd)->direntList[i].d_fileno)
                continue;
            if(!strcmp(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok))
            {
        log_msg("nphfuse_link: intermediate node found on offset %d\n",currentNode);
                npheap_unlock(nphfuse_data->devfd,currentNode);
                currentNode = ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno;
                found = 1;
                break;
            }
       /* if(((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno == 0) {
        npheap_unlock(nphfuse_data->devfd,currentNode);
        break;
        }*/
        }
        if(!found){
        if(strtok(NULL,"/")!= NULL){
                npheap_unlock(nphfuse_data->devfd,currentNode);
                return -ENOENT;
        }
        else {
        log_msg("nphfuse_link: Node not found. to be created\n");
           for(i=0;i<DIRENTNUM;i++) {
            if(!((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno)
                break;
           }
           if(i==DIRENTNUM) {
            log_msg("\nFolder full\n");
            npheap_unlock(nphfuse_data->devfd,currentNode);
            return -ENOMEM;
           }
           log_msg("nphfuse_link: creating a new link node Node number %d with name: %s and updating parent at %d\n",linkedNode,tok,i);
           ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno = linkedNode;
           strcpy(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok);
           //((struct nphfuse_dir*)(cwd))->direntList[i].d_type = DT_DIR;
           ((struct nphfuse_dir*)(cwd))->myStat.st_mtime = time(NULL);
           npheap_unlock(nphfuse_data->devfd,currentNode);
           log_msg("nphfuse_link: returning from mkdir\n");
           return 0;    //changed for testing

        }
        }
    else {
         npheap_unlock(nphfuse_data->devfd,currentNode);
    }
            tok = strtok(NULL,"/");

     }
	return -ENOENT;	
}

	/** Change the permission bits of a file */
int nphfuse_chmod(const char *path, mode_t mode){
    log_msg("nphfuse_chmod for path %s and mode is 0%03o\n",path,mode);
    char* tok = strtok(path,"/");
    int currentNode = ROOT_INODE;
    void* cwd =NULL;
//    tok = strtok(NULL,"/");
    while(tok!=NULL)
    {
        npheap_lock(nphfuse_data->devfd, currentNode);
        cwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
        int i=0;
        int found = 0;
        for(i=0;i<DIRENTNUM;i++)
        {
            if(!((struct nphfuse_dir*)cwd)->direntList[i].d_fileno)
                continue;
            if(!strcmp(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok))
            {
                npheap_unlock(nphfuse_data->devfd,currentNode);
                currentNode = ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno;
                found = 1;
                break;
            }
        }
        if(!found){
            log_msg("nphfuse_chmod: Inode not found\n");
            npheap_unlock(nphfuse_data->devfd,currentNode);
            return -ENOENT;
        }
        tok = strtok(NULL,"/");
     }
     log_msg("nphfuse_chmod: current node is %d\n",currentNode);
     npheap_lock(nphfuse_data->devfd, currentNode);
     void* fwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
     ((struct nphfuse_dir*)(fwd))->myStat.st_mode = mode;
     npheap_unlock(nphfuse_data->devfd, currentNode);
    return 0; //changed for testing
}

	/** Change the owner and group of a file */
int nphfuse_chown(const char *path, uid_t uid, gid_t gid){
	log_msg("nphfuse_chown for path %s and uid is %d and gid is %d\n",path,uid,gid);
    char* tok = strtok(path,"/");
    int currentNode = ROOT_INODE;
    void* cwd =NULL;
//    tok = strtok(NULL,"/");
    while(tok!=NULL)
    {
        npheap_lock(nphfuse_data->devfd, currentNode);
        cwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
        int i=0;
        int found = 0;
        for(i=0;i<DIRENTNUM;i++)
        {
            if(!((struct nphfuse_dir*)cwd)->direntList[i].d_fileno)
                continue;
            if(!strcmp(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok))
            {
                npheap_unlock(nphfuse_data->devfd,currentNode);
                currentNode = ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno;
                found = 1;
                break;
            }
        }
        if(!found){
            log_msg("nphfuse_chown: Inode not found\n");
            npheap_unlock(nphfuse_data->devfd,currentNode);
            return -ENOENT;
        }
        tok = strtok(NULL,"/");
     }
     log_msg("nphfuse_chown: current node is %d\n",currentNode);
     npheap_lock(nphfuse_data->devfd, currentNode);
     void* fwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
     ((struct nphfuse_dir*)(fwd))->myStat.st_uid = uid;
     if(gid !=-1)
	 	((struct nphfuse_dir*)(fwd))->myStat.st_gid = gid;
     npheap_unlock(nphfuse_data->devfd, currentNode);
    return 0; //changed for testing
}

	/** Change the size of a file */
int nphfuse_truncate(const char *path, off_t newsize){
	log_msg("nphfuse_truncate for path %s and size is %d\n",path,newsize);
	char* tok = strtok(path,"/");
    int currentNode = ROOT_INODE;
	void* cwd =NULL;
//    tok = strtok(NULL,"/");
    while(tok!=NULL)
    {
       	npheap_lock(nphfuse_data->devfd, currentNode);
    	cwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
    	int i=0;
    	int found = 0;
    	for(i=0;i<DIRENTNUM;i++)
    	{
            if(!((struct nphfuse_dir*)cwd)->direntList[i].d_fileno)
                continue;
        	if(!strcmp(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok))
        	{
        		npheap_unlock(nphfuse_data->devfd,currentNode);
        		currentNode = ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno;
        		found = 1;
        		break;
        	}
    	}
    	if(!found){
        	log_msg("nphfuse_truncate: Inode not found\n");
        	npheap_unlock(nphfuse_data->devfd,currentNode);
        	return -ENOENT;
    	}
    	tok = strtok(NULL,"/");
     }
     log_msg("nphfuse_truncate: current node is %d\n",currentNode);
     npheap_lock(nphfuse_data->devfd, currentNode);
     void* fwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
     ((struct nphfuse_dir*)(cwd))->myStat.st_size = newsize;
	 npheap_unlock(nphfuse_data->devfd, currentNode);
    return 0; //changed for testing
}

	/** Change the access and/or modification times of a file */
int nphfuse_utime(const char *path, struct utimbuf *ubuf)
{
	log_msg("nphfuse_utime for path %s\n",path);
	if(!strcmp(path,ROOT_DIR)){
		log_msg("\nnphfuse_utime: inside root directory\n");
		npheap_lock(nphfuse_data->devfd, ROOT_INODE);                          
        void* root = npheap_alloc(nphfuse_data->devfd, ROOT_INODE, BLOCK_SIZE);
		npheap_unlock(nphfuse_data->devfd, ROOT_INODE);
		log_msg("\nAbhash is returning root\n");
		return 0;   //testing to be changed to SUCCESS
    }
    char* tok = strtok(path,"/");
    int currentNode = ROOT_INODE;
	void* cwd =NULL;
	int i = 0;
//    tok = strtok(NULL,"/");
    while(tok!=NULL)
    {
    	npheap_lock(nphfuse_data->devfd, currentNode);                          
		cwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
		i=0;
		int found = 0;
		for(i=0;i<DIRENTNUM;i++)
		{
            if(!((struct nphfuse_dir*)cwd)->direntList[i].d_fileno)
                continue;
	    	if(!strcmp(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok))
	    	{
				npheap_unlock(nphfuse_data->devfd,currentNode);
				currentNode = ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno;
				found = 1;
				break;
	    	}
		}
		if(!found){
	    	log_msg("nphfuse_utime: Inode not found\n");
	    	npheap_unlock(nphfuse_data->devfd,currentNode);
	    	return -ENOENT;
		}
		tok = strtok(NULL,"/");
	
     }
     log_msg("nphfuse_utime: updating the access time and modification time %d for tok %s\n",currentNode,((struct nphfuse_dir*)(cwd))->direntList[i].d_name);
     npheap_lock(nphfuse_data->devfd, currentNode);
     void* fwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
	 ((struct nphfuse_dir*)fwd)->myStat.st_atime = ubuf->actime;
	 ((struct nphfuse_dir*)fwd)->myStat.st_mtime = ubuf->modtime;
     npheap_unlock(nphfuse_data->devfd, currentNode);
	 return 0;
	}

	/** File open operation
	 *
	 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
	 * will be passed to open().  Open should check if the operation
	 * is permitted for the given flags.  Optionally open may also
	 * return an arbitrary filehandle in the fuse_file_info structure,
	 * which will be passed to all file operations.
	 *
	 * Changed in version 2.2
	 */
	int nphfuse_open(const char *path, struct fuse_file_info *fi)
	{
	    log_msg("nphfuse_open: with path: %s\n",path);
//	    if ((fi->flags & O_ACCMODE) != O_RDONLY)
//		return -EACCES;

	    char* tok = strtok(path,"/");
    int currentNode = ROOT_INODE;
//    tok = strtok(NULL,"/");
    while(tok!=NULL)
    {   
        npheap_lock(nphfuse_data->devfd, currentNode);
        void* cwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
        int i=0;
        int found = 0;
        for(i=0;i<DIRENTNUM;i++)
        {   
            if(!((struct nphfuse_dir*)cwd)->direntList[i].d_fileno)
                        continue;
            if(!strcmp(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok))
            {   
                //npheap_unlock(nphfuse_data->devfd,currentNode);
                currentNode = ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno;
                found = 1;
                break;
            }
        }
        if(!found){
            log_msg("nphfuse_open: Inode not found\n");
            npheap_unlock(nphfuse_data->devfd,currentNode);
            //return empty statbuf
            //memset(stbuf,0,sizeof(struct stat));
            return -ENOENT;
        }
        tok = strtok(NULL,"/");
     
     }  
        log_msg("nphfuse_open: Node found. Setting fi->fh to %d\n", currentNode);
        fi->fh = currentNode;
        return 0;

}

	/** Read data from an open file
	 *
	 * Read should return exactly the number of bytes requested except
	 * on EOF or error, otherwise the rest of the data will be
	 * substituted with zeroes.  An exception to this is when the
	 * 'direct_io' mount option is specified, in which case the return
	 * value of the read system call will reflect the return value of
	 * this operation.
	 *
	 * Changed in version 2.2
	 */
	// I don't fully understand the documentation above -- it doesn't
	// match the documentation for the read() system call which says it
	// can return with anything up to the amount of data requested. nor
	// with the fusexmp code which returns the amount of data also
	// returned by read.
	int nphfuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
	{
	    log_msg("nphfuse_read: for path %s\n",path);
	    int file_ino = fi->fh;
	    log_msg("nphfuse_read: file_ino= %d and size is %d, offset %d\n",file_ino,size,offset);
	    npheap_lock(nphfuse_data->devfd,file_ino);
	    void* cwd = npheap_alloc(nphfuse_data->devfd, file_ino, BLOCK_SIZE);
	    int file_content = ((struct nphfuse_dir*)(cwd))->direntList[0].d_fileno;
	    int read_size = ((struct nphfuse_dir*)(cwd))->myStat.st_size;
		npheap_unlock(nphfuse_data->devfd,file_ino);
		log_msg("nphfuse_read: reading file at %d\n",file_content);
	    if(size+offset > BLOCK_SIZE){
			log_msg("nphfuse_read: NPHeap can't store this much data\n");
			return -EFAULT;
	    }
	    npheap_lock(nphfuse_data->devfd,file_content);
	    void* fwd_data = npheap_alloc(nphfuse_data->devfd, file_content, BLOCK_SIZE);
	    memcpy(buf,fwd_data+offset,read_size);
	    npheap_unlock(nphfuse_data->devfd,file_content);
		log_msg("Abhash:let's see what is read %s\n",buf);
	    return read_size;
	}

	/** Write data to an open file
	 *
	 * Write should return exactly the number of bytes requested
	 * except on error.  An exception to this is when the 'direct_io'
	 * mount option is specified (see read operation).
	 *
	 */
	int nphfuse_write(const char *path, const char *buf, size_t size, off_t offset,
		     struct fuse_file_info *fi)
	{
	    log_msg("nphfuse_write: for path %s\n",path);
        int file_ino = fi->fh;
        log_msg("nphfuse_write: file_ino= %d and size is %d, offset %d\n and data is %s\n",file_ino,size,offset,buf);
        npheap_lock(nphfuse_data->devfd,file_ino);
        void* cwd = npheap_alloc(nphfuse_data->devfd, file_ino, BLOCK_SIZE);
        int file_content = ((struct nphfuse_dir*)(cwd))->direntList[0].d_fileno;
		//update file size
		((struct nphfuse_dir*)(cwd))->myStat.st_size = offset+size;
		npheap_unlock(nphfuse_data->devfd,file_ino);
		log_msg("nphfuse_write: writting data at file_content %d\n",file_content);
        if(size+offset > BLOCK_SIZE){
        	log_msg("nphfuse_write: NPHeap can't store this much data\n");
        	return -EFAULT;
        }
        npheap_lock(nphfuse_data->devfd,file_content);
        void* fwd_data = npheap_alloc(nphfuse_data->devfd, file_content, BLOCK_SIZE);
        memcpy(fwd_data+offset,buf,size);
        npheap_unlock(nphfuse_data->devfd,file_content);
		log_msg("Data is written, return sucess\n");
        return size;
	}

	/** Get file system statistics
	 *
	 * The 'f_frsize', 'f_favail', 'f_fsid' and 'f_flag' fields are ignored
	 *
	 * Replaced 'struct statfs' parameter with 'struct statvfs' in
	 * version 2.5
	 */
	int nphfuse_statfs(const char *path, struct statvfs *statv)
	{
	    log_msg("nphfuse_statfs with path %s\n",path);
		int free_blocks = 0;
		int i =0 ;
		for(i=0;i<8192;i++){
			npheap_lock(nphfuse_data->devfd,i);
			int size = npheap_getsize(nphfuse_data->devfd,i);
			if(size == 0)
				free_blocks++;
			 npheap_unlock(nphfuse_data->devfd,i);
		}
		log_msg("nphfuse_statfs: free blocks are %d\n",free_blocks);
		statv->f_bsize = BLOCK_SIZE;
		statv->f_frsize = 1024;
		statv->f_blocks = (BLOCK_SIZE*8192)/1024;
		statv->f_bfree = free_blocks*8;
	    statv->f_bavail = free_blocks*8;
		statv->f_files = 8192 - ((free_blocks*2)-1);
		statv->f_ffree = free_blocks;
		statv->f_favail = free_blocks;
		statv->f_fsid = nphfuse_data->devfd;
		statv->f_namemax = 64;
		return 0;
	}

	/** Possibly flush cached data
	 *
	 * BIG NOTE: This is not equivalent to fsync().  It's not a
	 * request to sync dirty data.
	 *
	 * Flush is called on each close() of a file descriptor.  So if a
	 * filesystem wants to return write errors in close() and the file
	 * has cached dirty data, this is a good place to write back data
	 * and return any errors.  Since many applications ignore close()
	 * errors this is not always useful.
	 *
	 * NOTE: The flush() method may be called more than once for each
	 * open().  This happens if more than one file descriptor refers
	 * to an opened file due to dup(), dup2() or fork() calls.  It is
	 * not possible to determine if a flush is final, so each flush
	 * should be treated equally.  Multiple write-flush sequences are
	 * relatively rare, so this shouldn't be a problem.
	 *
	 * Filesystems shouldn't assume that flush will always be called
	 * after some writes, or that if will be called at all.
	 *
	 * Changed in version 2.2
	 */

	// this is a no-op in NPHFS.  It just logs the call and returns success
	int nphfuse_flush(const char *path, struct fuse_file_info *fi)
	{
	    log_msg("\nnphfuse_flush(path=\"%s\", fi=0x%08x)\n", path, fi);
	    // no need to get fpath on this one, since I work from fi->fh not the path
	    log_fi(fi);
		
	    return 0;
	}

	/** Release an open file
	 *
	 * Release is called when there are no more references to an open
	 * file: all file descriptors are closed and all memory mappings
	 * are unmapped.
	 *
	 * For every open() call there will be exactly one release() call
	 * with the same flags and file descriptor.  It is possible to
	 * have a file opened more than once, in which case only the last
	 * release will mean, that no more reads/writes will happen on the
	 * file.  The return value of release is ignored.
	 *
	 * Changed in version 2.2
	 */
	int nphfuse_release(const char *path, struct fuse_file_info *fi)
	{
	    log_msg("nphfuse_release: with path: %s\n",path);
	    npheap_unlock(nphfuse_data->devfd, fi->fh);
	    return 0;
	}

	/** Synchronize file contents
	 *
	 * If the datasync parameter is non-zero, then only the user data
	 * should be flushed, not the meta data.
	 *
	 * Changed in version 2.2
	 */
	int nphfuse_fsync(const char *path, int datasync, struct fuse_file_info *fi)
	{
	    log_msg("nphfuse_fsync for path %sa and datasync is %d\n",path,datasync);
		//There is nothing to be done as we are storing all metadata and data to npheap
		//which is persistant through it's lifycycle
	    return 0;
	}

	#ifdef HAVE_SYS_XATTR_H
	/** Set extended attributes */
	int nphfuse_setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
	{
	    log_msg("nphfuse_setxattr\n");
	    return -61;
	}

	/** Get extended attributes */
	int nphfuse_getxattr(const char *path, const char *name, char *value, size_t size)
	{
	    log_msg("nphfuse_getxattr\n");
	    return -61;
	}

	/** List extended attributes */
	int nphfuse_listxattr(const char *path, char *list, size_t size)
	{
	    log_msg("nphfuse_listxattr\n");
	    return -61;
	}

	/** Remove extended attributes */
	int nphfuse_removexattr(const char *path, const char *name)
	{
	log_msg("nphfuse_removexattr\n");
	    return -61;
	}
	#endif

	/** Open directory
	 *
	 * This method should check if the open operation is permitted for
	 * this directory
	 *
	 * Introduced in version 2.3
	 */
	int nphfuse_opendir(const char *path, struct fuse_file_info *fi)
	{
	    log_msg("nphfuse_opendir\n");
		
	char* tok = strtok(path,"/");
    int currentNode = ROOT_INODE;
//    tok = strtok(NULL,"/");
    while(tok!=NULL)
    {
        npheap_lock(nphfuse_data->devfd, currentNode);
        void* cwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
        int i=0;
        int found = 0;
        for(i=0;i<DIRENTNUM;i++)
        {
	    if(!((struct nphfuse_dir*)cwd)->direntList[i].d_fileno)
                        continue;
            if(!strcmp(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok))
            {
                //npheap_unlock(nphfuse_data->devfd,currentNode);
                currentNode = ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno;
                found = 1;
                break;
            }
        }
        if(!found){
            log_msg("nphfuse_opendir: Inode not found\n");
            npheap_unlock(nphfuse_data->devfd,currentNode);
            //return empty statbuf
            //memset(stbuf,0,sizeof(struct stat));
            return -ENOENT;
        }
        tok = strtok(NULL,"/");

     }
	log_msg("nphfuse_opendir: Node found. Setting fi->fh to %d\n", currentNode);
	fi->fh = currentNode;
	return 0;
}

	/** Read directory
	 *
	 * This supersedes the old getdir() interface.  New applications
	 * should use this.
	 *
	 * The filesystem may choose between two modes of operation:
	 *
	 * 1) The readdir implementation ignores the offset parameter, and
	 * passes zero to the filler function's offset.  The filler
	 * function will not return '1' (unless an error happens), so the
	 * whole directory is read in a single readdir operation.  This
	 * works just like the old getdir() method.
	 *
	 * 2) The readdir implementation keeps track of the offsets of the
	 * directory entries.  It uses the offset parameter and always
	 * passes non-zero offset to the filler function.  When the buffer
	 * is full (or an error happens) the filler function will return
	 * '1'.
	 *
	 * Introduced in version 2.3
	 */

int nphfuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
		       struct fuse_file_info *fi)
{
	log_msg("\nnphfuse_readdir: path: %s\n",path);
     log_msg("nphfuse_readdir: returning dir details for node:  %d\n",fi->fh);
     //npheap_lock(nphfuse_data->devfd, fi->fh);
     void* fwd = npheap_alloc(nphfuse_data->devfd, fi->fh, BLOCK_SIZE);
	 int i=0;
     for(i=0;i<DIRENTNUM;i++)
     {
		if(!((struct nphfuse_dir*)fwd)->direntList[i].d_fileno)
			continue;
     	if(filler(buf,((struct nphfuse_dir*)(fwd))->direntList[i].d_name ,NULL,0)) {
			log_msg("\nnphfuse_readdir buffer full. filler returned non zero\n");
			return -ENOMEM;
		}
     }
//	 npheap_unlock(nphfuse_data->devfd, fi->fh);
 
	 return 0;
}

	/** Release directory
	 */
	int nphfuse_releasedir(const char *path, struct fuse_file_info *fi)
	{
	    log_msg("nphfuse_releasedir: unlocking node fi->fh: %d\n",fi->fh);
	 npheap_unlock(nphfuse_data->devfd, fi->fh);
	    return 0;
	}

	/** Synchronize directory contents
	 *
	 * If the datasync parameter is non-zero, then only the user data
	 * should be flushed, not the meta data
	 *
	 * Introduced in version 2.3
	 */
	// when exactly is this called?  when a user calls fsync and it
	// happens to be a directory? ??? 
	int nphfuse_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
	{
            log_msg("nphfuse_fsyncdir\n");
	    return 0;
	}

int nphfuse_access(const char *path, int mask){
    log_msg("\nnphfuse_access: Inside path is %s mask is 0%o\n",path,mask);
    //return 0;
    if(!strcmp(path,ROOT_DIR))
    {
    	log_msg("\nnphfuse_access: inside root directory\n");
    	npheap_lock(nphfuse_data->devfd, ROOT_INODE);
    	void* root = npheap_alloc(nphfuse_data->devfd, ROOT_INODE, BLOCK_SIZE);
  	 	if ((((struct nphfuse_dir*)(root)))->myStat.st_mode & mask == mask){
        	 npheap_unlock(nphfuse_data->devfd, ROOT_INODE);
        	return 0; //changed for testing
    	}
    	npheap_unlock(nphfuse_data->devfd, ROOT_INODE);
    	return -1;
    }
    char* tok = strtok(path,"/");
    int currentNode = ROOT_INODE;
//    tok = strtok(NULL,"/");
    while(tok!=NULL)
    {
        npheap_lock(nphfuse_data->devfd, currentNode);
    void* cwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
    int i=0;
    int found = 0;
    for(i=0;i<DIRENTNUM;i++)
    {
        if(!((struct nphfuse_dir*)cwd)->direntList[i].d_fileno)
                continue;
        if(!strcmp(((struct nphfuse_dir*)(cwd))->direntList[i].d_name,tok))
        {
        npheap_unlock(nphfuse_data->devfd,currentNode);
        currentNode = ((struct nphfuse_dir*)(cwd))->direntList[i].d_fileno;
        found = 1;
        break;
        }
    }
    if(!found){
        log_msg("nphfuse_access: Inode not found\n");
        npheap_unlock(nphfuse_data->devfd,currentNode);
        //return empty statbuf
        return -ENOENT;
    }
    tok = strtok(NULL,"/");

     }
     log_msg("nphfuse_access: returning a new node with crtNode %d\n",currentNode);
     npheap_lock(nphfuse_data->devfd, currentNode);
     void* fwd = npheap_alloc(nphfuse_data->devfd, currentNode, BLOCK_SIZE);
     if ((((struct nphfuse_dir*)(fwd)))->myStat.st_mode & mask == mask){
    	 npheap_unlock(nphfuse_data->devfd, currentNode);
    	return 0; //changed for testing
	}
	npheap_unlock(nphfuse_data->devfd, currentNode);
    return -1;
}

/**
 * Change the size of an open file
 *
 * This method is called instead of the truncate() method if the
 * truncation was invoked from an ftruncate() system call.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the truncate() method will be
 * called instead.
 *
 * Introduced in version 2.5
 */
int nphfuse_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
{
    log_msg("nphfuse_ftruncate\n");
	//Get the index using fi->fh and update st_size
	
    return 0;
}

/**
 * Get attributes from an open file
 *
 * This method is called instead of the getattr() method if the
 * file information is available.
 *
 * Currently this is only called after the create() method if that
 * is implemented (see above).  Later it may be called for
 * invocations of fstat() too.
 *
 */
int nphfuse_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
	log_msg("nphfuse_fgetattr for path %s\n",path);
    
	return nphfuse_getattr(path, statbuf);
	
    return -ENOENT;
}

void *nphfuse_init(struct fuse_conn_info *conn)
{
    log_msg("\nnphfuse_init()\n");
    log_conn(conn);
    log_fuse_context(fuse_get_context());
    int size = npheap_getsize(nphfuse_data->devfd,ROOT_INODE);
    log_msg("\ninit: Root node size: %d\n",size);
    if(!size)
    {
	log_msg("\ninit: Creating root node in NPHEAP\n");
	npheap_lock(nphfuse_data->devfd, ROOT_INODE);
	void* root = npheap_alloc(nphfuse_data->devfd, ROOT_INODE, BLOCK_SIZE);
	struct nphfuse_dir rootNode;
	rootNode.myStat.st_ino = 1;
	rootNode.myStat.st_atime = time(NULL);
	rootNode.myStat.st_ctime = time(NULL);
	rootNode.myStat.st_mtime = time(NULL);
	rootNode.myStat.st_blksize = BLOCK_SIZE;
	rootNode.myStat.st_blocks = 8;
	rootNode.myStat.st_nlink = 2;
	rootNode.myStat.st_rdev = 0;
	rootNode.myStat.st_dev = nphfuse_data->devfd;
	rootNode.myStat.st_size = BLOCK_SIZE;
	rootNode.myStat.st_uid = getuid();
	rootNode.myStat.st_gid = getgid();
	rootNode.myStat.st_mode = S_IFDIR | 0755;
	int i =0;
	for(i=0;i<DIRENTNUM;i++)
	   rootNode.direntList[i].d_fileno = 0;
	memset(root,0,BLOCK_SIZE);
	memcpy(root,&rootNode, sizeof(rootNode));
	npheap_unlock(nphfuse_data->devfd, ROOT_INODE);
	log_msg("\ninit: Finished creating root node in NPHEAP\n");
    }
    return NPHFS_DATA;
}

/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 * Introduced in version 2.3
 */
void nphfuse_destroy(void *userdata)
{
    log_msg("\nnphfuse_destroy(userdata=0x%08x)\n", userdata);
}
