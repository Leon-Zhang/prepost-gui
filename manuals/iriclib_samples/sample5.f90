program Sample5
	implicit none
	include 'cgnslib_f.h'

	integer:: fin, ier, isize, jsize
	double precision:: time
	double precision, dimension(:,:), allocatable:: grid_x, grid_y

	! CGNS �t�@�C���̃I�[�v��
	call cg_open_f('test.cgn', CG_MODE_MODIFY, fin, ier)
	if (ier /=0) STOP "*** Open error of CGNS file ***"

	! iRIClib �̏�����
	call cg_iric_init_f(fin, ier)
	if (ier /=0) STOP "*** Initialize error of CGNS file ***"

	! �i�q�̃T�C�Y�𒲂ׂ�
	call cg_iric_gotogridcoord2d_f(isize, jsize, ier)
	! �i�q��ǂݍ��ނ��߂̃��������m��
	allocate(grid_x(isize,jsize), grid_y(isize,jsize))
	! �i�q��ǂݍ���
	call cg_iric_getgridcoord2d_f(grid_x, grid_y, ier)

	! ������Ԃ̏����o��
	time = 0

	call cg_iric_write_sol_time_f(time, ier)
	! �i�q���o��
	call cg_iric_write_sol_gridcoord2d_f (grid_x, grid_y, ier)

	do
		time = time + 10.0
		! (�����Ōv�Z�����s)
		call cg_iric_write_sol_time_f(time, ier)
		call cg_iric_write_sol_gridcoord2d_f (grid_x, grid_y, ier)
		If (time > 1000) exit
	end do

	! CGNS �t�@�C���̃N���[�Y
	call cg_close_f(fin, ier)
	stop
end program Sample5
