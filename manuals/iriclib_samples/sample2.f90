program Sample2
	implicit none
	include 'cgnslib_f.h'

	integer:: fin, ier, discharge_size, i
	double precision, dimension(:), allocatable:: discharge_time, discharge_value  ! discharge �̎����ƒl��ێ�����z��

	! CGNS �t�@�C���̃I�[�v��
	call cg_open_f('test.cgn', CG_MODE_MODIFY, fin, ier)
	if (ier /=0) STOP "*** Open error of CGNS file ***"

	! iRIClib �̏�����
	call cg_iric_init_f(fin, ier)
	if (ier /=0) STOP "*** Initialize error of CGNS file ***"

	! �܂��A�֐��^�̓��͏����̃T�C�Y�𒲂ׂ�
	call cg_iric_read_functionalsize_f('discharge', discharge_size, ier)
	! ���������m��
	allocate(discharge_time(discharge_size), discharge_value(discharge_size))
	! �m�ۂ����������ɒl��ǂݍ���
	call cg_iric_read_functional_f('discharge', discharge_time, discharge_value, ier)

	! �i�o�́j
	if (ier ==0) then
		print *, 'discharge: discharge_size=', discharge_size
		do i = 1, min(discharge_size, 5)
			print *, ' i,time,value:', i, discharge_time(i), discharge_value(i)
		end do
	end if

	! allocate �Ŋm�ۂ������������J��
	deallocate(discharge_time, discharge_value)

	! CGNS �t�@�C���̃N���[�Y
	call cg_close_f(fin, ier)
	stop
end program Sample2
