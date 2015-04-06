program SampleProgram
	implicit none
	include 'cgnslib_f.h'

	integer:: fin, ier
	integer:: icount, istatus
	integer:: i, j
	integer:: imax, jmax
	integer:: elev_on
	double precision:: elev_value
	double precision, dimension(:,:), allocatable::grid_x, grid_y, elevation

	character(200)::condFile

	icount = nargs()
	if ( icount.eq.2 ) then
		call getarg(1, condFile, istatus)
	else
		stop "Input File not specified."
	endif

	! �i�q�����f�[�^�t�@�C�����J��
	call cg_open_f(condFile, CG_MODE_MODIFY, fin, ier)
	if (ier /=0) stop "*** Open error of CGNS file ***"

	! iRIClib �̏������B�߂�l�� 1 �ɂȂ邪���Ȃ��B
	call cg_iric_init_f(fin, ier)

	! �i�q���������̓ǂݍ���
	! �Ȍ��ɋL�q���邽�߁A�G���[�����͍s���Ă��Ȃ�
	call cg_iric_read_integer_f("imax", imax, ier)
	call cg_iric_read_integer_f("jmax", jmax, ier)
	call cg_iric_read_integer_f("elev_on", elev_on, ier)
	call cg_iric_read_real_f("elev_value", elev_value, ier)

	! �G���[����
	if (imax * jmax > 100000 ) then
		! 100000 ���傫���i�q�͐����ł��Ȃ�
		call cg_iric_write_errorcode_f(1, ier)
		call cg_close_f(fin, ier)
		stop
  endif

	! �i�q�����p�̃��������m��
	allocate(grid_x(imax,jmax), grid_y(imax,jmax))
	allocate(elevation(imax,jmax))

	! �i�q�𐶐�
	do i = 1, imax
		do j = 1, jmax
			grid_x(i, j) = i
			grid_y(i, j) = j
			elevation(i, j) = elev_value
		end do
	end do

	! �i�q���o��
	call cg_iric_writegridcoord2d_f(imax, jmax, grid_x, grid_y, ier)
	if (elev_on == 1) then
		call cg_iric_write_grid_real_node_f("Elevation", elevation, ier);
	end if

	! �i�q�����f�[�^�t�@�C�������
 	call cg_close_f(fin, ier)
end program SampleProgram
