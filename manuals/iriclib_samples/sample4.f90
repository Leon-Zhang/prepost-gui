program Sample4
	implicit none
	include 'cgnslib_f.h'

	integer:: fin, ier, i
	double precision:: time

	! CGNS �t�@�C���̃I�[�v��
	call cg_open_f('test.cgn', CG_MODE_MODIFY, fin, ier)
	if (ier /=0) STOP "*** Open error of CGNS file ***"

	! iRIClib �̏�����
	call cg_iric_init_f(fin, ier)
	if (ier /=0) STOP "*** Initialize error of CGNS file ***"

	! ������Ԃ̏����o��
	time = 0

	call cg_iric_write_sol_time_f(time, ier)
	! (�����ŁA�����̌v�Z�i�q��v�Z���ʂ��o��)

	do
		time = time + 10.0
		! (�����Ōv�Z�����s)
		call cg_iric_write_sol_time_f(time, ier)
		! (�����ŁA�v�Z�i�q��v�Z���ʂ��o��)
		If (time > 1000) exit
	end do

	! CGNS �t�@�C���̃N���[�Y
	call cg_close_f(fin, ier)
	stop
end program Sample4
