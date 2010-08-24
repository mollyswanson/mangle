         Program fits2dat_binary
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! Angelica Costa Jan/03: From f90 to g77
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	 call go
	 end
	 
	 subroutine usage
	 implicit none
	 print *, 'fits2dat_binary.x Converts a map from fits to text'
	 print *, 'COMPILE: gfortran fits2dat_binary.f -o fits2dat_binary.x -lcfitsio -ffixed-line-length-none'  
	 print *, 'USAGE:   call fits2dat_binary.x <PolarType> <Nside> <infile> <outfile>'
	 print *, 'EXAMPLE: call fits2dat_binary.x 1 32 qaz_map.fits qaz_map.dat'
	 return
	 end

         subroutine go
         implicit  none
	 INTEGER  npixtot, nnpixtot, nmaps, nnmaps
	 parameter(nnpixtot=12*1024**2, nnmaps=4)
	 REAL     map(0:nnpixtot-1,1:nnmaps)
	 REAL     nullval
	 LOGICAL  anynull
	 CHARACTER*80 infile, outfile
	 INTEGER  status, unit, readwrite, blocksize, naxes(2), nfound, naxis
	 INTEGER  group, firstpix, nbuffer, npix, i, polar_type, Nside, lnblnk
	 REAL     blank, testval
	 REAL*8   bscale, bzero
	 LOGICAL  extend
	 INTEGER  nmove, hdutype
	 INTEGER  column, frow, imap
	 INTEGER  datacode, repeat, width
	 CHARACTER*80 comment
	 INTEGER  maxdim    !number of columns in the extension
	 PARAMETER(maxdim=20)
	 INTEGER  nrows, tfields, varidat
	 CHARACTER*20 ttype(maxdim), tform(maxdim), tunit(maxdim), extname

c       ----------------------------------------------------------------
c        get the arguments
c       ----------------------------------------------------------------
	 open (2,file='args.dat',status='old',err=777)
	 read (2,*,err=777,end=777) polar_type, Nside, infile, outfile
	 close(2)
	 npixtot = 12*Nside**2
	 if (npixtot.gt.nnpixtot) then
            write(0,*) 'ERROR: fits2dat_binary.f only supports resolution up to nside 1024.'
            write(0,*) 'If you need a higher resolution, edit nnpixtot in fits2dat_binary.f and recompile.'
            call usage
            STOP
         endif
	 if (polar_type.eq.1) then
	    nmaps = 1
	 else
	    nmaps = 4
	 endif
	 write(*,*) 'Polarization_Type_________________', polar_type 
	 write(*,*) 'Nside_____________________________', Nside
	 write(*,*) 'npix______________________________', npixtot 
	 write(*,*) 'Infile____________________________', infile (1:lnblnk( infile))
	 write(*,*) 'Outfile___________________________', outfile(1:lnblnk(outfile))
	
c       ----------------------------------------------------------------
c        loading the file
c       ----------------------------------------------------------------
	 status   = 0
	 unit     = 150
	 naxes(1) = 1
	 naxes(2) = 1
	 nfound   = -1
	 anynull  = .false.
	 bscale   = 1.0d0
	 bzero    = 0.0d0
	 blank    = -2.e25
	 nullval  = bscale*blank + bzero

	 readwrite=0
	 call ftopen(unit,infile,readwrite,blocksize,status)
	 if (status .gt. 0) call printerror(status)
	 !     -----------------------------------------

	 !     determines the presence of image
	 call ftgkyj(unit,'NAXIS', naxis, comment, status)
	 if (status .gt. 0) call printerror(status)

	 !     determines the presence of an extension
	 call ftgkyl(unit,'EXTEND', extend, comment, status)
	 if (status .gt. 0) status = 0 ! no extension : 
	 !     to be compatible with first version of the code

	 if (naxis .gt. 0) then ! there is an image
            !        determine the size of the image (look naxis1 and naxis2)
            call ftgknj(unit,'NAXIS',1,2,naxes,nfound,status)

            !        check that it found only NAXIS1
            if (nfound .eq. 2 .and. naxes(2) .gt. 1) then
               print *,'multi-dimensional image'
               print *,'expected 1-D data.'
               stop
            end if

            if (nfound .lt. 1) then
               call printerror(status)
               print *,'can not find NAXIS1.'
               stop
            endif

            npix=naxes(1)
            if (npix .ne. npixtot) then
               print *,'found ',npix,' pixels'
               print *,'expected ',npixtot
               stop
            endif

            call ftgkyd(unit,'BSCALE',bscale,comment,status)
            if (status .eq. 202) then ! BSCALE not found
               bscale = 1.0d0
               status = 0
            endif
            call ftgkyd(unit,'BZERO', bzero, comment,status)
            if (status .eq. 202) then ! BZERO not found
               bzero  = 0.0d0
               status = 0
            endif
            call ftgkye(unit,'BLANK', blank, comment,status)
            if (status .eq. 202) then ! BLANK not found 
               ! (according to fitsio BLANK is integer)
               blank  = -2.e25
               status = 0
            endif
            nullval = bscale*blank + bzero

            !        -----------------------------------------

            group    = 1 
            firstpix = 1
            call ftgpve
     &      (unit,group,firstpix,npix,nullval,map,anynull,status)
            ! if there are any NaN pixels, (real data)
            ! or BLANK pixels (integer data) they will take nullval value
            ! and anynull will switch to .true.
            ! otherwise, switch it by hand if necessary
            testval = 1.e-6 * ABS(nullval)
            do i=0, npix-1
               if (ABS(map(i,1)-nullval) .lt. testval) then
        	  anynull = .true.
c       	  goto 111
               endif
            enddo
c  111      continue

	 else if (extend) then ! there is an extension
            nmove = +1
            call ftmrhd(unit, nmove, hdutype, status)
            !cc         write(*,*) hdutype

            if (hdutype .ne. 2) then ! not a binary table
               stop 'this is not a binary table'
            endif

            !        reads all the keywords
            call ftghbn
     &      (unit, maxdim,nrows,tfields,ttype,tform,
     &      tunit,extname,varidat,status)

            if (tfields .lt. nmaps) then
               print *,'found ',tfields,' maps in the file'
               print *,'expected ',nmaps
               stop
            endif

            !        finds the bad data value
            call ftgkye(unit,'BAD_DATA',nullval,comment,status)
            if (status .eq. 202) then ! bad_data not found
               nullval = -1.6375e30 ! default value
               status  = 0
            endif

            do imap = 1, nmaps
               !parse TFORM keyword to find out the length of the column vector
               call ftbnfm(tform(imap), datacode, repeat, width, status)

               !reads the columns
               column   = imap
               frow     = 1
               firstpix = 1
               npix     = nrows * repeat
               if (npix .ne. npixtot) then
        	  print *,'found ',npix,' pixels'
        	  print *,'expected ',npixtot
        	  stop
               endif
               call ftgcve
     &         (unit, column, frow, firstpix, npix, nullval, 
     &         map(0,imap), anynull, status)
            enddo

	 else ! no image no extension, you are dead, man
            stop ' No image, no extension'
	 endif

	 !     close the file
	 call ftclos(unit, status)
	 
	 !     saving text file
	 print *, 'Saving ', outfile
         open (2,file=outfile)
         do i = 0, npix-1
	    write (2,*) (map(i,imap),imap=1,nmaps)
	 enddo
	 close(2)
	 
	 !     check for any error, and if so print out error messages
	 if (status .gt. 0) call printerror(status)
	 return
 777	 call usage
	 end	

      subroutine printerror(status)
      !=======================================================================
      !     Print out the FITSIO error messages to the user
      !=======================================================================
      implicit none
      INTEGER  status
      CHARACTER*30 errtext 
      CHARACTER*80 errmessage 
      !-----------------------------------------------------------------------
      !     check if status is OK (no error); if so, simply return
      if (status .le. 0)return

      !     get the text string which describes the error
      call ftgerr(status,errtext)
      print *,'FITSIO Error Status =',status,': ',errtext

      !     read and print out all the error messages on the FITSIO stack
      call ftgmsg(errmessage)
      do while (errmessage .ne. ' ')
         print *,errmessage
         call ftgmsg(errmessage)
      end do

      return
      end 

