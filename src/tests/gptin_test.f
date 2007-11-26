c-----------------------------------------------------------------------
c     © A J S Hamilton 2001
c-----------------------------------------------------------------------
      program testing
      implicit none
      real*8 rp(3,2),cm(2),rpi(3)
      integer gptin
      
c      cm(1) =  1.
c      cm(2) = -1.
c      rpi(1) = -0.866025404D0
c      rpi(2) =  1.06054021D-16
c      rpi(3) = -0.5
c      rp(1,1) = -1.D-16
c      rp(2,1) = -1.
c      rp(3,1) =  0.
c      rp(1,2) =  0.
c      rp(2,2) =  0.
c      rp(3,2) =  1.
      
c      cm(1) =  1.
c      cm(2) =  1.
c      rpi(1) = -1.
c      rpi(2) = -1.22460635D-16
c      rpi(3) =  0.
c      rp(1,1) = -1.D-16 
c      rp(2,1) = -1. 
c      rp(3,1) =  0.
c      rp(1,2) =  0. 
c      rp(2,2) =  0. 
c      rp(3,2) =  1.
c      

      cm(1) =  1. 
      cm(2) =  1.
      rpi(1) =  0.866025404D0
      rpi(2) = -0.5
      rpi(3) =  0.
      rp(1,1) = -1.D-16
      rp(2,1) = -1.
      rp(3,1) =  0.
      rp(1,2) =  0.
      rp(2,2) =  0.
      rp(3,2) =  1.
      
      if(gptin(rp,cm,2,rpi).eq.1) then
         write(*,*)"point in polygon"
      else 
         write(*,*)"point not in polygon"
      end if
      stop
      end
      


      logical function gptin(rp,cm,np,rpi)
      implicit none
      integer np
      real*8 rp(3,np),cm(np),rpi(3)
c     
c     intrinsics
      intrinsic abs
c     externals
      integer gzeroar
c     local (automatic) variables
      integer j
      real*8 cmij,cmj
c     *
c     * Determine whether unit direction rpi lies within region bounded by
c     *    1 - r.rp(j) <= cm(j)  (if cm(j).ge.0)
c     *    1 - r.rp(j) > -cm(j)  (if cm(j).lt.0)
c     * for j=1,np where rp(j) are unit directions.
c     *
c     Input: rp(3,j),j=1,np
c     cm(j),j=1,np
c     np
c     rpi(3)
c     Output: gptin = .true. if point lies within region
c     .false. if outside.
c     
      
      gptin=.false.
      write(*,*)"np =",np," cm(1) =",cm(1)," cm(2) =",cm(2)
      write(*,*)"rpi(1) =",rpi(1)," rpi(2) =",rpi(2)," rpi(3) =",rpi(3)
      write(*,*)"rp(1,1) =",rp(1,1)," rp(2,1) =",rp(2,1),
     &     " rp(3,1) =",rp(3,1)
      write(*,*)"rp(1,2) =",rp(1,2)," rp(2,2) =",rp(2,2),
     &     " rp(3,2) =",rp(3,2)
      

c     check for point outside because one circle is null
      if (gzeroar(cm,np).eq.0) goto 410
c     check each boundary
      do 140 j=1,np
c     null boundary means no constraint
         if (cm(j).ge.2.d0) goto 140
         cmj=abs(cm(j))
c     1-cos of angle between point and rp(j) direction
         cmij=((rpi(1)-rp(1,j))**2+(rpi(2)-rp(2,j))**2
     *        +(rpi(3)-rp(3,j))**2)/2.d0
c         write(*,*)"cmij =",cmij," cmj =",cmj
c         write(*,*)"foo"
         cmij=1.2345678987654321
         write(*,'(F20.9)')cmij
c         write(*,*)cmij
c     check if point is outside rp(j) boundary
         if (cm(j).ge.0.d0) then
c            write(*,*)"cm(",j,")>=0"
            if (.not.(cmij.le.cmj)) goto 410
            write(*,*)"cmij not gt cmj"
         elseif (cm(j).lt.0.d0) then
c           write(*,*)"foo" yes
            if (cmij.le.cmj) goto 410
         endif
 140  continue
c     point survived all assails
      gptin=.true.
      write(*,*)"point in polygon"
c     done
 410  continue
c      write(*,*)"cmij =",cmij," cmj =",cmj
      return
      end
c     
      integer function gzeroar(cm,np)
      integer np
      real*8 cm(np)
c
c        local (automatic) variables
      integer i
c *
c * Check for zero area because one circle is null.
c *
c  Input: cm
c         np
c Return value: 0 if area is zero because one circle is null,
c               1 otherwise
c
      do i=1,np
        if (cm(i).eq.0.d0) goto 200
        if (cm(i).le.-2.d0) goto 200
      enddo
      gzeroar=1
      return
c
  200 gzeroar=0
      return
c
      end
