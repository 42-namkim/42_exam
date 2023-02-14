#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

typedef	struct s_cmd
{
	int	pipe[2];
	int	is_pipe[2];
	int	p_pipe;
	char	**argv;
}	t_cmd;

void	ft_error(char *errmsg, char *info)
{
	if (errmsg)
		while(*errmsg)
			write(2, errmsg++, 1);
	if (info)
		while(*info)
			write(2, info++, 1);
	write(2, "\n", 1);
}

void	ft_fatal(void)
{
	write(2, "Fatal!!\n", 9);
	exit(1);
}

#include <stdio.h>
void	print_str_arr(char **argv)
{
	int idx = 0;
	while (argv[idx])
		printf("arg: %s\n", argv[idx++]);
}

int	get_idx(t_cmd *info, int start, char **argv)
{
	int		len;
	char	**args;

	args = &(argv[start]);
	len = 0;
	while (args[len] && strcmp(args[len], "|") && strcmp(args[len], ";"))
		len++;
	if (len > 0)
	{
		info->is_pipe[0] = info->is_pipe[1];
		if (args[len] && (strcmp(args[len], "|") == 0))
			info->is_pipe[1] = 1;
		else
			info->is_pipe[1] = 0;
		args[len] = NULL;
		info->argv = args;
	}
	return (start + len);	
}

void	exec_cmd(t_cmd *info, char **envp)
{
	pid_t	pid;
	int		ex_stat;

	info->p_pipe = info->pipe[0];
	if ((info->is_pipe[0] == 1 || info->is_pipe[1]) && pipe(info->pipe) == -1)
		ft_fatal();
	pid = fork();
	if (pid == -1)
		ft_fatal();
	if (pid == 0)
	{
		if (info->is_pipe[0] == 1)
			if (dup2(info->p_pipe, 0) == -1)
				ft_fatal();
		if (info->is_pipe[1] == 1)
			if (dup2(info->pipe[1], 1) == -1)
				ft_fatal();
		ex_stat = execve(info->argv[0], info->argv, envp);
		if (ex_stat == -1)
			ft_error("cannot execute ", info->argv[0]);
		exit(ex_stat);
	}
	else
	{
		waitpid(pid, NULL, 0);
		if (info->is_pipe[0] == 1 || info->is_pipe[1])
		{
			close(info->pipe[1]);
			if (info->is_pipe[1] != 1)
				close(info->pipe[0]);
			if (info->is_pipe[0] == 1)
				close(info->p_pipe);
		}
	}
}

int	main(int argc, char **argv, char **envp)
{
	t_cmd	info;
	int		i;
	int		start;

	i = 1;
	while (i < argc)
	{
		start = i;
		i = get_idx(&info, start, argv);
		if (strcmp(argv[start], "cd") == 0)
		{
			if (i - start != 2)	//
				ft_error("Error: wrong args", NULL);
			else if (chdir(argv[start + 1]))
				ft_error("cannot go to ", argv[start + 1]);
		}
		else
			exec_cmd(&info, envp);
		i++;
		while (argv[i] && (strcmp(argv[i], ";") == 0))
			i++;
	}	
	return (0);
}
