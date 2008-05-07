      Program dat2fits_binary
      call go
      end

	subroutine usage
	implicit none
	print *, 'Converts a CMBfast test file (or mangle healpix_weight file) into a fits file'
c	print *, 'COMPILE: a f 'f77  dat2fits_binary.f -o dat2fits_binary.x libcfitsio.a'  
	print *, 'USAGE:   call dat2fits_binary.x <ptype> <nside> <in> <out>'
	print *, 'EXAMPLE: call dat2fits_binary.x 1 512 qaz_map_data.dat mytest.fits'
	return
	end
      
      !=======================================================================
      subroutine go
      !=======================================================================
      implicit none
      integer   polar_type, nmap, nside, npix, nnpix
      integer   nlheader, i, j
      parameter(nnpix=12*1024**2, nlheader=80)
      real      map(0:nnpix,4) 
      character*80 header(nlheader), infile, outfile, card

c     ----------------------------------------------------------------  
c     get the arguments 					        
c     ----------------------------------------------------------------  
      open (2,file='args.dat',status='old',err=777)		        
      read (2,*,err=777,end=777) polar_type, nside, infile, outfile  
      close(2)  						        
      npix = 12*nside**2
      if (npix.gt.nnpix) pause 'npix TOO LARGE'					        
      if (polar_type.eq.1) then 				        
    	 nmap = 1						        
      else							        
    	 nmap = 4						        
      endif							        
      write(*,*) 'Polar_Type____', polar_type			        
      write(*,*) 'nmap__________', nmap 			        
      write(*,*) 'Nside_________', nside			        
      write(*,*) 'Npix__________', npix 			        
      write(*,*) 'Infile________', infile (1:lnblnk( infile))	        
      write(*,*) 'Outfile_______', outfile(1:lnblnk(outfile))	        

c     ----------------------------------------------------------------  
c     loading the ascii file					        
c     ----------------------------------------------------------------  
      print *, 'Loading ', infile(1:lnblnk(infile))		        
      open (2,file=infile)					        
      do i=0,npix-1						        
    	 read (2,*) (map(i,j),j=1,nmap) 			        
      enddo							        
      close(2)  						        

c     ----------------------------------------------------------------  
c     writing the header 					        
c     ---------------------------------------------------------------- 
      do i=1,nlheader 
         header(i) = ' '
      enddo
      call a_add_card(header,'COMMENT','---------------------','')
      call a_add_card(header,'COMMENT',' Sky Map Keywords    ','')
      call a_add_card(header,'COMMENT','---------------------','')
      call a_add_card
     &     (header,'PIXTYPE' ,'HEALPIX','HEALPIX Pixelisation'   )
      call a_add_card
     &     (header,'ORDERING','NESTED'   ,'Pixel ordering scheme'  )
      call i_add_card
     &     (header,'NSIDE'   ,nside    ,'Resolution for HEALPIX' )
      call i_add_card
     &     (header,'FIRSTPIX',0        ,'First pixel # (0 based)')
      call i_add_card
     &     (header,'LASTPIX' ,npix-1   ,'Last  pixel # (0 based)')
      call v_add_card(header)  
      call a_add_card(header,'COMMENT','----------------','')
      call a_add_card(header,'COMMENT',' Data Keywords  ','')
      call a_add_card(header,'COMMENT','----------------','')
      call a_add_card
     &     (header,'TTYPE1', 'TEMPERATURE','Temperature map')
      call a_add_card(header,'TUNIT1', 'unknown','map unit')
      call v_add_card(header)
      if (polar_type.ne.1) then
	 call a_add_card
     &   (header,'TTYPE2', 'Q-POLARIZATION','Q Polar map')
	 call a_add_card
     &   (header,'TUNIT2', 'unknown','map unit')
	 call v_add_card(header)
	 call a_add_card
     &   (header,'TTYPE3', 'U-POLARIZATION','U Polar map')
	 call a_add_card
     &   (header,'TUNIT3', 'unknown','map unit')
	 call v_add_card(header)
      endif
      
      
c      nlheader = SIZE(header)
c      print *, 'Writing Header' 
c      call headerLine('COMMENT ------------------',hd(1))
c      call headerLine('COMMENT = Healpix'         ,hd(2))
c      call headerLine('COMMENT ------------------',hd(3))
c      call headerLine('PIXTYPE  = HEALPIX',hd(4))
c      call headerLine('ORDERING = NESTED'   ,hd(5))
c      call headerLine(card,hd(6))
c      write (card,'("NSIDE",i8," /Resolution for HEALPIX")') nside
c      call headerLine(card,hd(7))
c      write (card,'("FIRSTPIX",i8," /First pixel")') 0
c      call headerLine(card,hd(8))
c      write (card,'("LASTPIX" ,i16," /Last  pixel")') npix-1
c      call headerLine('COMMENT ------------------',hd(9))
c      call headerLine('COMMENT = Data Description',hd(10))
c      call headerLine('COMMENT ------------------',hd(11))
c      call headerLine('TTYPE1 = TEMPERATURE',hd(12))
c      call headerLine('TUNIT1 = unknown'    ,hd(13))
c      nh = 13
c      if (polar_type.ne.1) then
c	call headerLine('TTYPE2 = Q-POLARIZATION',hd(14))
c	call headerLine('TUNIT2 = unknown'       ,hd(15))
c	call headerLine('TTYPE3 = U-POLARIZATION',hd(16))
c	call headerLine('TUNIT3 = unknown'       ,hd(17))
c	nh = 17
c      endif
      
      
      do i=1,nlheader 
         print *, header(i)
      enddo
      print *, 'Saving file'
      call write_bintab (map,npix,nmap,header,nlheader,outfile)
      return
 777  call usage
      end  
   
      SUBROUTINE a_add_card(header, kwd, value, comment) ! character
      CHARACTER*(*) value
      CHARACTER*80  header
      CHARACTER*(*) kwd
      CHARACTER*(*) comment
      CHARACTER*240 st_value, st_comment
      st_value   = ''
      st_comment = ''
      write(st_value,  '(a)')   value
      write(st_comment,'(a)') comment
      call write_hl(header, kwd, st_value, st_comment)
      RETURN
      END 

      SUBROUTINE v_add_card(header) ! blank line
      CHARACTER*80 header
      call write_hl(header, 'COMMENT', ' ', ' ')
      END 

      SUBROUTINE i_add_card(header, kwd, value, comment) ! integer (i*4)
      INTEGER       value
      CHARACTER*80  header
      CHARACTER*(*) kwd
      CHARACTER*(*) comment
      CHARACTER*20  st_value
      write(st_value,'(i20)') value
      call write_hl(header, kwd, st_value, comment)
      RETURN
      END 

      SUBROUTINE write_hl(header, kwd, st_value, comment)
      IMPLICIT  none
      CHARACTER*80  header(80)
      CHARACTER*(*) kwd
      CHARACTER*(*) comment
      CHARACTER*(*) st_value
      INTEGER       hdtype, status
      INTEGER       iw, lnblnk
      CHARACTER*240 headerline
      CHARACTER*80  buffheader
      CHARACTER*10  pad10	 
      iw = 1
      do while(header(iw) /= '')
	 iw = iw + 1
      enddo
      pad10=''
      buffheader =''
      headerline = kwd     (1:lnblnk(kwd))     //' '//
     &             st_value(1:lnblnk(st_value))//' '//
     &             comment (1:lnblnk(comment))
      if (headerline .eq. 'COMMENT') then ! COMMENT alone
	 header(iw) = 'COMMENT'
	 iw = iw + 1
	 RETURN
      endif
      hdtype = 0
      status = 0
      CALL ftgthd(headerline(1:79), buffheader, hdtype, status)
      header(iw) = buffheader 
      if (len_trim(headerline) > 79) then
	 status = 0
	 CALL ftgthd(pad10//headerline(80:149), buffheader, hdtype, status)
	 iw = iw + 1
	 header(iw) = buffheader
      endif
      if (len_trim(headerline) > 149) then
	 status = 0
	 CALL ftgthd(pad10//headerline(150:219), buffheader, hdtype, status)
	 iw = iw + 1
	 header(iw) = buffheader
      endif
      if (len_trim(headerline) > 219) then
	 status = 0
	 CALL ftgthd(pad10//headerline(220:240), buffheader, hdtype, status)
	 iw = iw + 1
	 header(iw) = buffheader
      endif
      iw = iw + 1
      RETURN
      END  

      !=======================================================================
      subroutine write_bintab (map,npix,nmap,header,nlheader,filename)
      !=======================================================================
      !     Create a FITS file containing a binary table extension with 
      !     the temperature map in the first column
      !     written by EH from writeimage and writebintable 
      !     (fitsio cookbook package)
      !
      !     slightly modified to deal with vector column (ie TFORMi = '1024E')
      !     in binary table       EH/IAP/Jan-98
      !
      !     simplified the calling sequence, the header sould be filled in
      !     before calling the routine
      !=======================================================================
      IMPLICIT none

      INTEGER       npix, nmap, nlheader
      REAL          map(0:npix-1,1:nmap) 
      CHARACTER*80  header(1:nlheader) 
      CHARACTER*(*) filename

      INTEGER       status,unit,blocksize,bitpix,naxis,naxes(1)
      INTEGER       group,fpixel,nelements,i
      LOGICAL       simple,extend
      CHARACTER*80  svalue, comment
      REAL*8        bscale,bzero

      INTEGER       maxdim	     !number of columns in the extension
      PARAMETER    (maxdim = 20)
      INTEGER       nrows, tfields, varidat
      INTEGER       frow,  felem, colnum
      CHARACTER*20  ttype(maxdim), tform(maxdim), tunit(maxdim), extname
      CHARACTER*8   date
      CHARACTER*10  fulldate
      CHARACTER*10  card
      CHARACTER*2   stn
      INTEGER       itn

      !-----------------------------------------------------------------------

      status= 0
      unit  = 100

      !     ----------------------
      !     create the new empty FITS file
      !     ----------------------
      blocksize=1
      call ftinit(unit,filename,blocksize,status)

      !     ----------------------
      !     initialize parameters about the FITS image
      !     ----------------------
      simple  =.true.
      bitpix  =32     ! integer*4
      naxis   =0      ! no image
      naxes(1)=0
      extend  =.true. ! there is an extension

      !     ----------------------
      !     primary header
      !     ----------------------
      !     write the required header keywords
      call ftphpr
     &(unit,simple,bitpix,naxis,naxes,0,1,extend,status)

      !     writes supplementary keywords : none

      !     write the current date
      call ftpdat(unit,status) ! format (dd/mm/yy)

      !     update the date (format ccyy-mm-dd)
      call date_and_time(date)
      fulldate = date(1:4)//'-'//date(5:6)//'-'//date(7:8)
      comment = 'FITS file creation date ccyy-mm-dd'
      call ftukys(unit,'DATE',fulldate,comment,status)

      !     ----------------------
      !     image : none
      !     ----------------------

      !     ----------------------
      !     extension
      !     ----------------------

      !     creates an extension
      call ftcrhd(unit, status)

      !     writes required keywords
      nrows    = npix / 1024 ! naxis1
      tfields  = nmap
      do i=1,nmap
	 tform(i) = '1024E'
	 if (npix .lt. 1024) then ! for nside <= 8
            nrows = npix
            tform(i) = '1E'
	 endif
	 ttype(i) = 'simulation'   ! will be updated
	 tunit(i) = ''             ! optional, will not appear
      enddo
      extname  = ''                ! optional, will not appear
      varidat  = 0
      call ftphbn
     &(unit,nrows,tfields,ttype,tform,tunit,extname,varidat,status)

      !     write the header literally, putting TFORM1 at the desired place
      do i=1,nlheader
         card = header(i)
         if (card(1:5) == 'TTYPE') then ! if TTYPE1 is explicitely given
            stn = card(6:6)
            read(stn,'(i1)') itn
            ! discard at their original location:
            call ftmcrd(unit,'TTYPE'//stn,'COMMENT',status)  ! old TTYPEi and 
            call ftmcrd(unit,'TFORM'//stn,'COMMENT',status)  !     TFORMi
            call ftprec(unit,header(i), status)              ! write new TTYPE1
            comment = 'data format of field: 4-byte REAL'
            call ftpkys(unit,'TFORM'//stn,tform(1),comment,status) ! and write new TFORM1 right after
            elseif (header(i).NE.' ') then
            call ftprec(unit,header(i), status)
         endif
 10      continue
      enddo

      !     write the extension one column by one column
      frow   = 1  ! starting position (row)
      felem  = 1  ! starting position (element)
      do colnum = 1, nmap
         call ftpcle
     &   (unit,colnum,frow,felem,npix,map(0,colnum),status)
      enddo

      !     ----------------------
      !     close and exit
      !     ----------------------
      call ftclos(unit, status)

      !     ----------------------
      !     check for any error, and if so print out error messages
      !     ----------------------
      if (status .gt. 0) call printerror(status)

      return
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

c --------------------------------------------------------------------
c return formatted header line
c --------------------------------------------------------------------
      subroutine headerLine(line,hdLine)
      implicit none
      integer hdtype,status
      character*(*) line
      character*80 hdLine

      hdtype=0
      status=0
      call ftgthd(line,hdLine,hdtype,status)
      if (status.gt.0) call errorMessage(status)
      end

c --------------------------------------------------------------------
c print error message and stop
c --------------------------------------------------------------------
      subroutine errorMessage(status)
      implicit none
      integer status
      character*30 errmsg

      call ftgerr(status,errmsg)
      print '("FITS error ",i3,": ",a)', status,errmsg
      stop
      end
