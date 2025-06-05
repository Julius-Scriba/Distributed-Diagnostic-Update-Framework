import { useQuery } from '@tanstack/react-query';
import api from '../api';

export interface CommandEntry {
  command: string;
  parameters?: Record<string, unknown>;
  timestamp?: string;
}

interface CommandsResponse {
  commands: CommandEntry[];
}

export default function useCommands(uuid: string | null) {
  return useQuery({
    queryKey: ['commands', uuid],
    queryFn: async () => {
      const { data } = await api.get<CommandsResponse>(`/admin/commands/${uuid}`);
      return data.commands;
    },
    enabled: !!uuid,
  });
}
